// EPOS Cortex-A SETUP

#include <system/config.h>

extern "C" { void _vector_table() __attribute__ ((used, naked, section(".init"))); }

// Interrupt Vector Table
void _vector_table()
{
    ASM("\t\n\
        b   _reset                                                           \t\n\
        b   _undefined_instruction                                           \t\n\
        b   _software_interrupt                                              \t\n\
        b   _prefetch_abort                                                  \t\n\
        b   _data_abort                                                      \t\n\
        nop                           // Reserved                            \t\n\
        b   _irq                                                             \t\n\
        b   _fiq  

        _reset:
        MRC     p15, 4, r2, c15, c0, 0  
        LDR     r3, [r2, #0x208]        
        BIC     r3, r3, #0x01           
        STR     r3, [r2, #0x208]        

        CMP     r0, #0                  
        MOVNE   r0, #0x00               
        MOVEQ   r0, #0x04               
        STR     r0, [r2, #0x208]        

      
      STREQ   r1, [r2, #0x218]                                   
      
      
      MOV     r0, #0x0
      STR     r0, [r2, #0x0]
      STR     r0, [r2, #0x4]
                                                           \t\n\
        ");
}
