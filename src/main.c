#include "ch32fun.h"

#define SERVO_PIN 2
#define SERVO_FRAME_US 20000
#define SERVO_MIN_US 1000
#define SERVO_MAX_US 1500
#define SERVO_STEP_US 5
#define SERVO_HOLD_FRAMES 25

static void servo_frame(unsigned int pulse_us)
{
    GPIOD->BSHR = 1U << SERVO_PIN;
    Delay_Us(pulse_us);
    GPIOD->BSHR = 1U << (16 + SERVO_PIN);
    Delay_Us(SERVO_FRAME_US - pulse_us);
}

int main(void)
{
    SystemInit();

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;
    GPIOD->BSHR = 1U << (16 + SERVO_PIN);
    // Configure only PD2; preserve PD1 for programming/debugging.
    GPIOD->CFGLR = (GPIOD->CFGLR & ~(0xFU << (SERVO_PIN * 4)))
                  | (0x3U << (SERVO_PIN * 4));

    while (1)
    {
        for (unsigned int pulse_us = SERVO_MIN_US;
             pulse_us < SERVO_MAX_US; pulse_us += SERVO_STEP_US)
        {
            servo_frame(pulse_us);
        }
        for (unsigned int frame = 0; frame < SERVO_HOLD_FRAMES; ++frame)
        {
            servo_frame(SERVO_MAX_US);
        }
        for (unsigned int pulse_us = SERVO_MAX_US;
             pulse_us > SERVO_MIN_US; pulse_us -= SERVO_STEP_US)
        {
            servo_frame(pulse_us);
        }
        for (unsigned int frame = 0; frame < SERVO_HOLD_FRAMES; ++frame)
        {
            servo_frame(SERVO_MIN_US);
        }
    }
}
