// EPOS Cortex-A SETUP

#include <system/config.h>

extern "C" { void _vector_table() __attribute__ ((used, naked, section(".init"))); }

// Interrupt Vector Table
void _vector_table()
{
    ASM("\t\n\
        b   _reset                                                                                              \t\n\
        b   _undefined_instruction                                                                              \t\n\
        b   _software_interrupt                                                                                 \t\n\
        b   _prefetch_abort                                                                                     \t\n\
        b   _data_abort                                                                                         \t\n\
        nop                           // Reserved                                                               \t\n\
        b   _irq                                                                                                \t\n\
        b   _fiq                                                                                                \t\n\
                                                                                                                \t\n\
    _reset:                                                                                                     \t\n\
                                                                                                                \t\n\
        //Based on moodle example:                                                                              \t\n\
                                                                                                                \t\n\
        //Generic initialization for all CPUs                                                                   \t\n\
                                                                                                                \t\n\
        //MMU init                                                                                              \t\n\
        //Invalidate caches                                                                                     \t\n\
                                                                                                                \t\n\
        MOV     r0, #0                                                                                          \t\n\
        MCR     p15, 0, r0, c7, c5, 0     // ICIALLU - Invalidate entire I Cache, and flushes branch target cache \t\n\
                                                                                                                \t\n\
        MRC     p15, 1, r0, c0, c0, 1     // Read CLIDR                                                         \t\n\
        ANDS    r3, r0, #07000000                                                                               \t\n\
        MOV     r3, r3, LSR #23           // Cache level value (naturally aligned)                              \t\n\
        BEQ     invalidate_caches_finished                                                                      \t\n\
        MOV     r10, #0                                                                                         \t\n\
                                                                                                                \t\n\
    invalidate_caches_loop1:                                                                                    \t\n\
        ADD     r2, r10, r10, LSR #1      // Work out 3xcachelevel                                              \t\n\
        MOV     r1, r0, LSR r2            // bottom 3 bits are the Cache type for this level                    \t\n\
        AND     r1, r1, #7                // get those 3 bits alone                                             \t\n\
        CMP     r1, #2                                                                                          \t\n\
        BLT     invalidate_caches_skip    // no cache or only instruction cache at this level                   \t\n\
        MCR     p15, 2, r10, c0, c0, 0    // write the Cache Size selection register                            \t\n\
        ISB                               // ISB to sync the change to the CacheSizeID reg                      \t\n\
        MRC     p15, 1, r1, c0, c0, 0     // reads current Cache Size ID Register                               \t\n\
        AND     r2, r1, #7               // extract the line length field                                      \t\n\
        ADD     r2, r2, #4                // add 4 for the line length offset (log2 16 bytes)                   \t\n\
        LDR     r4, =0x3FF                                                                                      \t\n\
        ANDS    r4, r4, r1, LSR #3        // R4 is the max number on the way size (right aligned)               \t\n\
        CLZ     r5, r4                    // R5 is the bit position of the way size increment                   \t\n\
        LDR     r7, =0x00007FFF                                                                                 \t\n\
        ANDS    r7, r7, r1, LSR #13       // R7 is the max number of the index size (right aligned)             \t\n\
                                                                                                                \t\n\
    invalidate_caches_loop2:                                                                                    \t\n\
        MOV     r9, R4                    // R9 working copy of the max way size (right aligned)                \t\n\
                                                                                                                \t\n\
    invalidate_caches_loop3:                                                                                    \t\n\
        ORR     r11, r10, r9, LSL r5      // factor in the way number and cache number into r11                 \t\n\
        ORR     r11, r11, r7, LSL r2      // factor in the index number                                         \t\n\
        MCR     p15, 0, r11, c7, c6, 2    // DCISW - invalidate by set/way                                      \t\n\
        SUBS    r9, r9, #1                // decrement the way number                                           \t\n\
        BGE     invalidate_caches_loop3                                                                         \t\n\
        SUBS    r7, r7, #1                // decrement the index                                                \t\n\
        BGE     invalidate_caches_loop2                                                                         \t\n\
                                                                                                                \t\n\
    invalidate_caches_skip:                                                                                     \t\n\
        ADD     r10, r10, #2              // increment the cache number                                         \t\n\
        CMP     r3, r10                                                                                         \t\n\
        BGT     invalidate_caches_loop1                                                                         \t\n\
                                                                                                                \t\n\
        //Clear Branch Prediction array                                                                         \t\n\
        MOV     r0, #0x0                                                                                        \t\n\
        MCR     p15, 0, r0, c7, c5, 6     // BPIALL - Invalidate entire branch predictor array                  \t\n\
                                                                                                                \t\n\
        //Invalidate TLBs                                                                                       \t\n\
        MOV     r0, #0x0                                                                                        \t\n\
        MCR     p15, 0, r0, c8, c7, 0     // TLBIALL - Invalidate entire Unifed TLB                             \t\n\
                                                                                                                \t\n\
        //STANDARD ENTRIES                                                                                      \t\n\
                                                                                                                \t\n\
        //Entry for VA 0x0                                                                                      \t\n\
        //This region must be coherent                                                                          \t\n\
        LDR     r1, =0x0                  // Physical address                                                   \t\n\
        LDR     r2, =0x00014c06           // Descriptor template                                                \t\n\
        ORR     r1, r1, r2                // Combine address and template                                       \t\n\
        STR     r1, [r0]                                                                                        \t\n\
                                                                                                                \t\n\
        // Entry for VA 0x0010,0000                                                                             \t\n\
        // Each CPU stores private data in this address range                                                   \t\n\
        // Using the MMU to map to different PA on each CPU.                                                    \t\n\
                                                                                                                \t\n\
        // CPU 0 - PA Base                                                                                      \t\n\
        // CPI 1 - PA Base + 1MB                                                                                \t\n\
        // CPU 2 - PA Base + 2MB                                                                                \t\n\
        // CPU 3 - PA Base + 3MB                                                                                \t\n\
                                                                                                                \t\n\
        MRC     p15, 0, r1, c0, c0, 5     // Re-read Multiprocessor Affinity Register                           \t\n\
        AND     r1, r1, #0x03             // Mask off, leaving the CPU ID field                                 \t\n\
        MOV     r1, r1, LSL #20           // Convert core ID into a MB offset                                   \t\n\
                                                                                                                \t\n\
        LDR     r3, =0x00100000           // Base PA                                                            \t\n\
        ADD     r1, r1, r3                // Add CPU offset to PA                                               \t\n\
        LDR     r2, =0x00000c1e           // Descriptor template                                                \t\n\
        ORR     r1, r1, r2                // Combine address and template                                       \t\n\
        STR     r1, [r0, #4]                                                                                    \t\n\
                                                                                                                \t\n\
        // Entry for private address space                                                                       \t\n\
        // Needs to be marked as Device memory                                                                   \t\n\
        MRC     p15, 4, r1, c15, c0, 0    // Get base address of private address space                          \t\n\
        LSR     r1, r1, #20               // Clear bottom 20 bits, to find which 1MB block its in               \t\n\
        LSL     r2, r1, #2                // Make a copy, and multiply by four.  This gives offset into the page tables     \t\n\
        LSL     r1, r1, #20               // Put back in address format                                         \t\n\
                                                                                                                \t\n\
        LDR     r3, =0x00000c06           // Descriptor template                                                \t\n\
        ORR     r1, r1, r3                // Combine address and template                                       \t\n\
        STR     r1, [r0, r2]                                                                                    \t\n\
                                                                                                                \t\n\
        DSB                                                                                                     \t\n\
                                                                                                                \t\n\
        //Enable MMU                                                                                            \t\n\
        MRC     p15, 0, r0, c1, c0, 0     // Read current control reg                                           \t\n\
        ORR     r0, r0, #0x01             // Set M bit                                                          \t\n\
        MCR     p15, 0, r0, c1, c0, 0     // Write reg back                                                     \t\n\
                                                                                                                \t\n\
        b _start                                                                                                \t\n\
                                                                                                                \t\n\
        ");

}


