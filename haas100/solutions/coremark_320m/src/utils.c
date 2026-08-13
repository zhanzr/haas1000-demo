#include <aos/kernel.h>
#include <stdarg.h>
#include <stdio.h>
#include "utils.h"

void TICK_Init(void)
{
    /* AliOS Things tick (1 kHz) already runs; nothing to do. */
}

uint32_t HAL_GetTick(void)
{
    return (uint32_t)aos_now_ms();
}

void HAL_Delay(uint32_t t)
{
    aos_msleep(t);
}

void uart_printf(const char *fmt, ...)
{
    char buf[512];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    printf("%s", buf);
}
