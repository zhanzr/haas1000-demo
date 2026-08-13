/*
 * CoreMark 1.0.1 for the HaaS100 board (HaaS1000 SoC, Cortex-M33 @ ~320 MHz).
 * Ported from the STM32H723 reference (coremark_550m) to AliOS Things:
 *   - timing via HAL_GetTick() -> aos_now_ms()
 *   - CPU frequency measured with hal_sys_timer_calc_cpu_freq()
 */
#include <stdio.h>
#include <aos/kernel.h>
#include "custom_def.h"
#include "core_portme.h"
#include "hal_timer.h"

int coremark_main(void);

int application_start(int argc, char *argv[])
{
    uint32_t cpu_hz = hal_sys_timer_calc_cpu_freq(5, 0);

    while (1) {
        printf("\r\n--- CoreMark run on HaaS100 (Cortex-M33) @ %lu Hz ---\r\n",
               (unsigned long)cpu_hz);
        coremark_main();
        printf("--- CoreMark complete. %lu Hz, %s ---\r\n",
               (unsigned long)cpu_hz, COMPILER_NAME);
        for (int i = 0; i < 10; i++) {
            aos_msleep(1000);
        }
    }

    return 0;
}
