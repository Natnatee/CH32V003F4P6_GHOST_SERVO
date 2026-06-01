#include "ch32fun.h"
#include <stdio.h>

#define LED_PORT GPIOD
#define LED_PIN  4  // PD4 is Physical Pin 1

int main()
{
    // Initialize internal 24MHz clock
    SystemInit();

    // Initialize SWIO print terminal
    SetupDebugPrintf();

    // Enable clock gating for GPIOD
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOD;

    // Configure PD4 as Output Push-Pull 50MHz (0x3)
    // Mask out bits 16-19 for Pin 4, then OR it with 0x3
    // This strictly preserves PD1 (SWIO) settings
    LED_PORT->CFGLR = (LED_PORT->CFGLR & ~(0xF << (LED_PIN * 4))) | (0x3 << (LED_PIN * 4));

    printf("==================================================\r\n");
    printf("        CH32V003 DEDICATED PD4 BLINK TEST         \r\n");
    printf("==================================================\r\n");
    printf("Target: PD4 (Physical Pin 1 on TSSOP20)\r\n");
    printf("Status: Running...\r\n\r\n");

    while(1)
    {
        // Turn LED ON
        printf("[Status] PD4 -> HIGH (ON)\r\n");
        LED_PORT->BSHR = 1 << LED_PIN;
        Delay_Ms(1000);

        // Turn LED OFF
        printf("[Status] PD4 -> LOW  (OFF)\r\n");
        LED_PORT->BSHR = 1 << (16 + LED_PIN);
        Delay_Ms(1000);
    }
}