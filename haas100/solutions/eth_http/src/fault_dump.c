/*
 * fault_dump.c: minimal hard fault reporter for debugging eth_http.
 * Overrides the weak HardFault_Handler so a crash prints the stacked
 * registers instead of silently spinning forever.
 */
#include <stdio.h>
#include <stdint.h>

static void dump_frame(const uint32_t *sp, const char *which)
{
    printf("[fault] %s frame\r\n", which);
    printf("[fault] R0=0x%08x R1=0x%08x R2=0x%08x R3=0x%08x\r\n",
           sp[0], sp[1], sp[2], sp[3]);
    printf("[fault] R12=0x%08x LR=0x%08x PC=0x%08x xPSR=0x%08x\r\n",
           sp[4], sp[5], sp[6], sp[7]);
}

__attribute__((naked, noinline)) void HardFault_Handler(void)
{
    __asm volatile(
        "mrs r0, msp\n"
        "mrs r1, psp\n"
        "mov r2, lr\n"
        "bl  _fault_dump_c\n"
        "b .\n");
}

__attribute__((noinline)) void _fault_dump_c(uint32_t msp, uint32_t psp, uint32_t exc_lr)
{
    printf("[fault] HardFault EXC_RETURN=0x%08x\r\n", exc_lr);
    dump_frame((const uint32_t *)psp, "PSP");
    dump_frame((const uint32_t *)msp, "MSP");
}
