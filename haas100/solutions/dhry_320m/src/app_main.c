/*
 * Dhrystone 2.1 for the HaaS100 board (HaaS1000 SoC, Cortex-M33 @ ~320 MHz).
 * Ported from the STM32H723 reference (dhry_550m) to AliOS Things:
 *   - timing via HAL_GetTick() -> aos_now_ms()
 *   - CPU frequency measured with hal_sys_timer_calc_cpu_freq()
 */
#include <stdio.h>
#include <aos/kernel.h>
#include "custom_def.h"
#include "dhry.h"
#include "hal_timer.h"

int application_start(int argc, char *argv[])
{
    uint32_t cpu_hz = hal_sys_timer_calc_cpu_freq(5, 0);

    printf("\r\n=== Dhrystone 2.1 on HaaS100 (Cortex-M33) @ %lu Hz ===\r\n",
           (unsigned long)cpu_hz);

    while (1) {
        dhry_main(cpu_hz);
        printf("\r\nCPU freq: %lu Hz (%lu MHz)\r\n",
               (unsigned long)cpu_hz, (unsigned long)(cpu_hz / 1000000UL));
        printf("Compiler: %s\r\n", COMPILER_NAME);
        aos_msleep(10000);
    }

    return 0;
}
