#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>

#define RAM_FUNC

#define PRINTF(...)  printf(__VA_ARGS__)

void TICK_Init(void);
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t t);
void uart_printf(const char *fmt, ...);

#endif
