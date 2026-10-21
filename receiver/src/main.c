#include "ch32fun.h"
#include <stdint.h>
#include <stdio.h>

#define IR_PIN 4
#define SERVO_PIN 2

#define COMMAND_WORD 0xA5C33CU
#define TICKS_US ((uint32_t)DELAY_US_TIME)
// About 160 degrees using the observed ~90 degrees per 1000us span.
// Shift both endpoints rather than extending the upper pulse beyond 2500us.
#define HOME_US 600U
#define OUT_US 2380U
#define STEP_US 10U
#define FRAME_US 20000U

typedef enum { SERVO_READY, MOVING_OUT, HOLDING, MOVING_HOME, COOLDOWN, START_HOME } servo_state;

static uint8_t ir_previous;
static uint32_t ir_edge_ticks;
static uint32_t ir_high_us;
static uint32_t ir_word;
static uint8_t ir_bits;
static uint8_t ir_synced;
static uint8_t ir_confirmed;
static uint32_t ir_confirmation_ticks;
static uint32_t ir_edges;
static uint32_t ir_syncs;
static uint32_t ir_packets;
static uint32_t ir_errors;
static uint32_t ir_matches;
static uint32_t ir_last_word;
#ifdef IR_TEST
static uint32_t ir_low_us;
static uint32_t ir_error_high;
static uint32_t ir_error_low;
static unsigned int ir_error_bits;
static unsigned int ir_max_bits;
static uint32_t ir_high_bins[4];
static uint32_t ir_low_bins[4];
static unsigned int pulse_bin(uint32_t us)
{
    return us < 100 ? 0 : us <= 600 ? 1 : us <= 1300 ? 2 : 3;
}
#endif
static char log_buffer[480];
static unsigned int log_length;
static unsigned int log_position;

// Same seven-byte SDI format as the SDK, but never wait for the host.
static void log_poll(void)
{
    if (log_position >= log_length || (*DMDATA0 & 0x80U))
        return;
    unsigned int count = log_length - log_position;
    if (count > 7)
        count = 7;
    uint32_t data0 = 0x80U | (count + 4U);
    uint32_t data1 = 0;
    for (unsigned int i = 0; i < count; ++i)
    {
        uint32_t value = (unsigned char)log_buffer[log_position + i];
        if (i < 3)
            data0 |= value << (8U * (i + 1U));
        else
            data1 |= value << (8U * (i - 3U));
    }
    *DMDATA1 = data1;
    *DMDATA0 = data0;
    log_position += count;
}

static int between(uint32_t value, uint32_t low, uint32_t high)
{
    return value >= low && value <= high;
}

// A demodulating 38kHz IR receiver is LOW during a mark, HIGH at idle.
// Decode its inverted envelope without blocking servo generation.
static int ir_poll(uint32_t now, int enabled)
{
    uint8_t level = ((GPIOC->INDR & (1U << IR_PIN)) == 0U);
    if (!enabled)
    {
        ir_synced = 0;
        ir_confirmed = 0;
    }

    if (level == ir_previous)
        return 0;

    ++ir_edges;
    uint32_t duration_us = (uint32_t)(now - ir_edge_ticks) / TICKS_US;
    ir_edge_ticks = now;
    ir_previous = level;
    if (!level)
    {
        ir_high_us = duration_us;
#ifdef IR_TEST
        ++ir_high_bins[pulse_bin(duration_us)];
#endif
        return 0;
    }
#ifdef IR_TEST
    ir_low_us = duration_us;
    ++ir_low_bins[pulse_bin(duration_us)];
#endif
    if (!enabled)
        return 0;

    if (between(duration_us, 6500, 13000) && between(ir_high_us, 100, 600))
    {
        ++ir_syncs;
        ir_synced = 1;
        ir_bits = 0;
        ir_word = 0;
        return 0;
    }
    if (!ir_synced)
        return 0;

    unsigned int bit;
    // Allow receiver pulse distortion, but reject near-equal/noisy pairs.
    uint32_t total_us = ir_high_us + duration_us;
    int valid_pair = between(total_us, 900, 1500) &&
                     between(ir_high_us, 80, 1300) &&
                     between(duration_us, 80, 1300);
    if (valid_pair && ir_high_us * 5U <= duration_us * 4U)
        bit = 0;
    else if (valid_pair && duration_us * 5U <= ir_high_us * 4U)
        bit = 1;
    else
    {
        ++ir_errors;
#ifdef IR_TEST
        ir_error_high = ir_high_us;
        ir_error_low = duration_us;
        ir_error_bits = ir_bits;
#endif
        ir_synced = 0;
        return 0;
    }
    ir_word = (ir_word << 1) | bit;
    ++ir_bits;
#ifdef IR_TEST
    if (ir_bits > ir_max_bits)
        ir_max_bits = ir_bits;
#endif
    if (ir_bits != 24)
        return 0;
    ir_synced = 0;
    ++ir_packets;
    ir_last_word = ir_word;
    if (ir_word != COMMAND_WORD)
    {
        ir_confirmed = 0;
        return 0;
    }
    int matched = ir_confirmed &&
                  (uint32_t)(now - ir_confirmation_ticks) < 500000U * TICKS_US;
    ir_confirmation_ticks = now;
    ir_confirmed = !matched;
    if (matched)
        ++ir_matches;
    return matched;
}

#ifndef IR_TEST
int main(void)
{
    SystemInit();
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
    GPIOC->CFGLR = (GPIOC->CFGLR & ~(0xFU << (IR_PIN * 4)))
                  | (0x4U << (IR_PIN * 4)); // PC4 floating input

    GPIOD->BSHR = 1U << (16 + SERVO_PIN);
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xFU << (SERVO_PIN * 4)))
                  | (0x3U << (SERVO_PIN * 4));

    uint32_t now = funSysTick32();
    uint32_t frame_ticks = now;
    uint32_t state_ticks = now;

    int logged_state = -1;
    uint32_t logged_matches = 0;
    uint8_t startup_test = 1;
    uint32_t pulse_us = HOME_US;
    uint8_t pulse_high = 1;
    servo_state state = START_HOME;
    ir_previous = ((GPIOC->INDR & (1U << IR_PIN)) == 0U);
    ir_edge_ticks = now;
    GPIOD->BSHR = 1U << SERVO_PIN;

    while (1)
    {
        now = funSysTick32();
        if (ir_poll(now, state == SERVO_READY))
            state = MOVING_OUT;

        if (state == START_HOME && (uint32_t)(now - state_ticks) >= 1000000U * TICKS_US)
            state = MOVING_OUT;
        if (state == HOLDING && (uint32_t)(now - state_ticks) >= 5000000U * TICKS_US)
            state = MOVING_HOME;
        if (state == COOLDOWN && (uint32_t)(now - state_ticks) >= 10000000U * TICKS_US)
            state = SERVO_READY;

        if (pulse_high && (uint32_t)(now - frame_ticks) >= pulse_us * TICKS_US)
        {
            GPIOD->BSHR = 1U << (16 + SERVO_PIN);
            pulse_high = 0;
        }
        if ((uint32_t)(now - frame_ticks) >= FRAME_US * TICKS_US)
        {
            if (state == MOVING_OUT)
            {
                pulse_us += STEP_US;
                if (pulse_us >= OUT_US)
                {
                    pulse_us = OUT_US;
                    state = HOLDING;
                    state_ticks = now;
                }
            }
            else if (state == MOVING_HOME)
            {
                pulse_us -= STEP_US;
                if (pulse_us <= HOME_US)
                {
                    pulse_us = HOME_US;
                    state = startup_test ? SERVO_READY : COOLDOWN;
                    startup_test = 0;
                    state_ticks = now;
                }
            }
            frame_ticks = now;
            GPIOD->BSHR = 1U << SERVO_PIN;
            pulse_high = 1;
        }

        // Format only during the low pulse and outside an in-progress IR frame.
        if (!pulse_high)
        {
            log_poll();
            if (!ir_synced && log_position >= log_length &&
                (logged_state != (int)state || logged_matches != ir_matches) &&
                (uint32_t)(now - frame_ticks) < 15000U * TICKS_US)
            {
                const char *names[] = { "READY", "OUT", "HOLD", "HOME", "WAIT10", "START_HOME" };
                const char *event = logged_matches != ir_matches ? "CONFIRMED A5C33C" : "STATE";
                int length = snprintf(log_buffer, sizeof(log_buffer),
                    "[IR RX] %s test=%u state=%s pulse=%lu match=%lu\r\n",
                    event, (unsigned int)startup_test, names[state],
                    (unsigned long)pulse_us, (unsigned long)ir_matches);
                logged_state = (int)state;
                logged_matches = ir_matches;
                log_length = length > 0 ? (unsigned int)length : 0;
                if (log_length >= sizeof(log_buffer))
                    log_length = sizeof(log_buffer) - 1;
                log_position = 0;
            }
        }
    }
}
#else
int main(void)
{
    SystemInit();
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC | RCC_APB2Periph_GPIOD;
    GPIOC->CFGLR = (GPIOC->CFGLR & ~(0xFU << (IR_PIN * 4)))
                  | (0x4U << (IR_PIN * 4));
    // No servo pulses in IR-only mode; keep the signal safely LOW.
    GPIOD->BSHR = 1U << (16 + SERVO_PIN);
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xFU << (SERVO_PIN * 4)))
                  | (0x3U << (SERVO_PIN * 4));
    uint32_t now = funSysTick32();
    uint32_t report_ticks = now;
    uint32_t previous_edges = 0;
    ir_previous = ((GPIOC->INDR & (1U << IR_PIN)) == 0U);
    ir_edge_ticks = now;
    while (1)
    {
        now = funSysTick32();
        ir_poll(now, 1);
        log_poll();
        if (log_position >= log_length &&
            (uint32_t)(now - report_ticks) >= 1000000U * TICKS_US)
        {
            // One bounded snapshot per second, never printf per edge.
            uint32_t elapsed_ms = (uint32_t)(now - report_ticks) / (1000U * TICKS_US);
            report_ticks = now;
            int length = snprintf(log_buffer, sizeof(log_buffer),
                "[IRTEST] dt=%lums mark=%u edges=%lu dE=%lu sync=%lu bad=%lu packets=%lu last=%06lX match=%lu maxbits=%u\r\n"
                "[US] H=%lu L=%lu failH=%lu failL=%lu bits=%u Hbin=%lu,%lu,%lu,%lu Lbin=%lu,%lu,%lu,%lu\r\n",
                (unsigned long)elapsed_ms, (unsigned int)ir_previous,
                (unsigned long)ir_edges, (unsigned long)(ir_edges - previous_edges),
                (unsigned long)ir_syncs, (unsigned long)ir_errors,
                (unsigned long)ir_packets, (unsigned long)ir_last_word,
                (unsigned long)ir_matches, ir_max_bits,
                (unsigned long)ir_high_us, (unsigned long)ir_low_us,
                (unsigned long)ir_error_high, (unsigned long)ir_error_low, ir_error_bits,
                (unsigned long)ir_high_bins[0], (unsigned long)ir_high_bins[1],
                (unsigned long)ir_high_bins[2], (unsigned long)ir_high_bins[3],
                (unsigned long)ir_low_bins[0], (unsigned long)ir_low_bins[1],
                (unsigned long)ir_low_bins[2], (unsigned long)ir_low_bins[3]);
            previous_edges = ir_edges;
            log_length = length > 0 ? (unsigned int)length : 0;
            if (log_length >= sizeof(log_buffer))
                log_length = sizeof(log_buffer) - 1;
            log_position = 0;
        }
    }
}
#endif
