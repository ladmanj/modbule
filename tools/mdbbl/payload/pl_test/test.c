#include <stdint.h>
#include "iostruct.h"
#define __NOP()	__asm volatile ("nop")
int bl_func(uint8_t *data) __attribute__ ( (aligned (4) ) );

int bl_func(uint8_t *data)
{
    holding_regs *regs = (holding_regs*)data;
    int i;
    for(i=0;i<100000;i++)
    {
	    __NOP();
    }
    regs->rmt_command = 0x21756143;
}
