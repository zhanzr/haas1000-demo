#include <aos/kernel.h>
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
