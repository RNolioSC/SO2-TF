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
    _irq:                                                                                                       \t\n\
        SUB     lr, lr, #4          // Pre-adjust lr                                                            \t\n\
        SRSFD   r13!, #0x12          // Save lr and SPRS to IRQ mode stack                                       \t\n\
        PUSH    {r0-r4, r12}        // Sace APCS corruptable registers to IRQ mode stack (and maintain 8 byte alignment) \t\n\
                                                                                                                \t\n\
        // Read Acknowledge of the interrupt                                                                    \t\n\
        MRC     p15, 4, r0, c15, c0, 0  // Read periph base address                                             \t\n\
        LDR     r0, [r0, #0x010C]       // Read the Interrupt Acknowledge Register  (ICCIAR)                    \t\n\
        MOV     r4, r0                                                                                          \t\n\
                                                                                                                \t\n\
        // This example only uses (and enables) one.  At this point                                             \t\n\
        // you would normally check the ID, and clear the source.                                               \t\n\
                                                                                                                \t\n\
        // Write end of interrupt reg                                                                            \t\n\
        MOV     r0, r4                                                                                          \t\n\
        MOV     r1, r0                  // Back up passed in ID value                                           \t\n\
        MRC     p15, 4, r0, c15, c0, 0  // Read periph base address                                             \t\n\
        STR     r1, [r0, #0x0110]       // Write ID to the End of Interrupt register (ICCEOIR)                  \t\n\
                                                                                                                \t\n\
        POP     {r0-r4, r12}        // Restore stacked APCS registers                                           \t\n\
        MOV     r2, #0x01           // Set r2 so CPU leaves holding pen                                         \t\n\
        RFEFD   r13!                 // Return from exception                                                    \t\n\
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
        AND     r2, r1, #7               // extract the line length field                                       \t\n\
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
    invalidate_caches_finished:                                                                                 \t\n\
                                                                                                                \t\n\
        //Clear Branch Prediction array                                                                         \t\n\
        MOV     r0, #0x0                                                                                        \t\n\
        MCR     p15, 0, r0, c7, c5, 6     // BPIALL - Invalidate entire branch predictor array                  \t\n\
                                                                                                                \t\n\
        //Invalidate TLBs                                                                                       \t\n\
        MOV     r0, #0x0                                                                                        \t\n\
        MCR     p15, 0, r0, c8, c7, 0     // TLBIALL - Invalidate entire Unifed TLB                             \t\n\
                                                                                                                \t\n\
        //TODO: page tables for each CPU                                                                        \t\n\
                                                                                                                \t\n\
        //STANDARD ENTRIES                                                                                      \t\n\
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
        //BRANCH PREDICTION INIT                                                                                \t\n\
        MRC     p15, 0, r0, c1, c0, 0     // Read SCTLR                                                         \t\n\
        ORR     r0, r0, #(1 << 11)        // Set the Z bit (bit 11)                                             \t\n\
        MCR     p15, 0,r0, c1, c0, 0      // Write SCTLR                                                        \t\n\
                                                                                                                \t\n\
        //Init global timer                                                                                     \t\n\
        // Get base address of private perpherial space                                                         \t\n\
        MRC     p15, 4, r2, c15, c0, 0  // Read periph base address                                              \t\n\
        // Ensure the timer is disabled                                                                         \t\n\
        LDR     r3, [r2, #0x208]        // Read control reg                                                     \t\n\
        BIC     r3, r3, #0x01           // Clear enable bit                                                     \t\n\
        STR     r3, [r2, #0x208]        // Write control reg                                                    \t\n\
        // Form control reg value                                                                               \t\n\
        CMP     r0, #0                  // Check whether to enable auto-reload                                  \t\n\
        MOVNE   r0, #0x00               // No auto-reload                                                       \t\n\
        MOVEQ   r0, #0x04               // With auto-reload                                                     \t\n\
        STR     r0, [r2, #0x208]        // Store to control register                                            \t\n\
        // Store increment value                                                                                \t\n\
        STREQ   r1, [r2, #0x218]                                                                                \t\n\
        // Clear timer value                                                                                    \t\n\
        MOV     r0, #0x0                                                                                        \t\n\
        STR     r0, [r2, #0x0]                                                                                  \t\n\
        STR     r0, [r2, #0x4]                                                                                  \t\n\
                                                                                                                \t\n\
        //SMP INITIALIZATION                                                                                    \t\n\
        MRC     p15, 0, r0, c0, c0, 5     // Read CPU ID register                                               \t\n\
        ANDS    r0, r0, #0x03             // Mask off, leaving the CPU ID field                                 \t\n\
        BLEQ    primary_cpu_init                                                                                \t\n\
        BLNE    secondary_cpus_init                                                                             \t\n\
                                                                                                                \t\n\
        //PRIMARY CPU INIT (CPU 0)                                                                              \t\n\
    primary_cpu_init:                                                                                           \t\n\
                                                                                                                \t\n\
        //Enable SCU                                                                                            \t\n\
        MRC     p15, 4, r0, c15, c0, 0  // Read periph base address                                             \t\n\
        LDR     r1, [r0, #0x0]          // Read the SCU Control Register                                        \t\n\
        ORR     r1, r1, #0x1            // Set bit 0 (The Enable bit)                                           \t\n\
        STR     r1, [r0, #0x0]          // Write back modifed value                                             \t\n\
                                                                                                                \t\n\
        //Join SMP                                                                                              \t\n\
        MOV     r0, #0x0                  // Move CPU ID into r0                                                \t\n\
        MOV     r1, #0xF                  // Move 0xF (represents all four ways) into r1                        \t\n\
                                                                                                                \t\n\
        //Secure SCU invalidate                                                                                 \t\n\
        AND     r0, r0, #0x03           // Mask off unused bits of CPU ID                                       \t\n\
        MOV     r0, r0, LSL #2          // Convert into bit offset (four bits per core)                         \t\n\
        AND     r1, r1, #0x0F           // Mask off unused bits of ways                                         \t\n\
        MOV     r1, r1, LSL r0          // Shift ways into the correct CPU field                                \t\n\
        MRC     p15, 4, r2, c15, c0, 0  // Read periph base address                                             \t\n\
        STR     r1, [r2, #0x0C]         // Write to SCU Invalidate All in Secure State                          \t\n\
                                                                                                                \t\n\
        //Now is joining                                                                                        \t\n\
        MRC     p15, 0, r0, c1, c0, 1   // Read ACTLR                                                           \t\n\
        ORR     r0, r0, #0x040          // Set bit 6                                                            \t\n\
        MCR     p15, 0, r0, c1, c0, 1   // Write ACTLR                                                          \t\n\
                                                                                                                \t\n\
        //Maintenance broadcast                                                                                 \t\n\
        MRC     p15, 0, r0, c1, c0, 1   // Read Aux Ctrl register                                               \t\n\
        ORR     r0, r0, #0x01           // Set the FW bit (bit 0)                                               \t\n\
        MCR     p15, 0, r0, c1, c0, 1   // Write Aux Ctrl register                                              \t\n\
                                                                                                                \t\n\
        //GIC INIT                                                                                              \t\n\
        //Enable GIC                                                                                            \t\n\
        MRC     p15, 4, r0, c15, c0, 0  // Read periph base address                                             \t\n\
        ADD     r0, r0, #0x1000         // Add the GIC offset                                                   \t\n\
        LDR     r1, [r0]                // Read the GIC's Enable Register  (ICDDCR)                             \t\n\
        ORR     r1, r1, #0x01           // Set bit 0, the enable bit                                            \t\n\
        STR     r1, [r0]                // Write the GIC's Enable Register  (ICDDCR)                            \t\n\
                                                                                                                \t\n\
        //GIC processor interface enable                                                                        \t\n\
        MRC     p15, 4, r0, c15, c0, 0  // Read periph base address                                             \t\n\
        LDR     r1, [r0, #0x100]        // Read the Processor Interface Control register   (ICCICR/ICPICR)      \t\n\
        ORR     r1, r1, #0x03           // Bit 0: Enables secure interrupts, Bit 1: Enables Non-Secure interrupts \t\n\
        STR     r1, [r0, #0x100]        // Write the Processor Interface Control register  (ICCICR/ICPICR)      \t\n\
                                                                                                                \t\n\
        b _start                                                                                                \t\n\
                                                                                                                \t\n\
    secondary_cpus_init:                                                                                        \t\n\
                                                                                                                \t\n\
        //GIC processor interface enable                                                                        \t\n\
        MRC     p15, 4, r0, c15, c0, 0  // Read periph base address                                             \t\n\
        LDR     r1, [r0, #0x100]        // Read the Processor Interface Control register   (ICCICR/ICPICR)      \t\n\
        ORR     r1, r1, #0x03           // Bit 0: Enables secure interrupts, Bit 1: Enables Non-Secure interrupts \t\n\
        STR     r1, [r0, #0x100]        // Write the Processor Interface Control register  (ICCICR/ICPICR)      \t\n\
                                                                                                                \t\n\
        //Priority mask                                                                                         \t\n\
        MOV     r0, #0x1F               // Priority                                                             \t\n\
        MOV     r1, r0                  // Back up passed in ID value                                           \t\n\
        MRC     p15, 4, r0, c15, c0, 0  // Read periph base address                                             \t\n\
        STR     r1, [r0, #0x0104]       // Write the Priority Mask register (ICCPMR/ICCIPMR)                    \t\n\
                                                                                                                \t\n\
        MOV     r0, #0x0                  // ID                                                                 \t\n\
        //Get base address of private perpherial space                                                          \t\n\
        MOV     r1, r0                  // Back up passed in ID value                                           \t\n\
        MRC     p15, 4, r0, c15, c0, 0  // Read periph base address                                             \t\n\
        // Each interrupt source has an enable bit in the GIC.  These                                           \t\n\
        // are grouped into registers, with 32 sources per register                                             \t\n\
        // First, we need to identify which 32 bit block the interrupt lives in                                 \t\n\
        MOV     r2, r1                  // Make working copy of ID in r2                                        \t\n\
        MOV     r2, r2, LSR #5          // LSR by 5 places, affective divide by 32                              \t\n\
                                        // r2 now contains the 32 bit block this ID lives in                    \t\n\
        MOV     r2, r2, LSL #2          // Now multiply by 4, to covert offset into an address offset (four bytes per reg) \t\n\
        // Now work out which bit within the 32 bit block the ID is                                             \t\n\
        AND     r1, r1, #0x1F           // Mask off to give offset within 32bit block                           \t\n\
        MOV     r3, #1                  // Move enable value into r3                                            \t\n\
        MOV     r3, r3, LSL r1          // Shift it left to position of ID                                      \t\n\
        ADD     r2, r2, #0x1100         // Add the base offset of the Enable Set registers to the offset for the ID \t\n\
        STR     r3, [r0, r2]            // Store out  (ICDISER)                                                 \t\n\
                                                                                                                \t\n\
        MOV     r0, #0x0                  // ID                                                                 \t\n\
        MOV     r1, #0x0                  // Priority                                                           \t\n\
        // Get base address of private perpherial space                                                         \t\n\
        MOV     r2, r0                  // Back up passed in ID value                                           \t\n\
        MRC     p15, 4, r0, c15, c0, 0  // Read periph base address                                             \t\n\
        // Make sure that priority value is only 5 bits, and convert to expected format                         \t\n\
        AND     r1, r1, #0x1F                                                                                   \t\n\
        MOV     r1, r1, LSL #3                                                                                  \t\n\
        // Find which priority register this ID lives in                                                        \t\n\
        BIC     r3, r2, #0x03           // Make a copy of the ID, clearing off the bottom two bits              \t\n\
        // There are four IDs per reg, by clearing the bottom two bits we get an address offset                 \t\n\
        ADD     r3, r3, #0x1400         // Now add the offset of the Priority Level registers from the base of the private peripheral space \t\n\
        ADD     r0, r0, r3              // Now add in the base address of the private peripheral space, giving us the absolute address \t\n\
        // Now work out which ID in the register it is                                                          \t\n\
        AND     r2, r2, #0x03           // Clear all but the bottom four bits, leaves which ID in the reg it is (which byte) \t\n\
        MOV     r2, r2, LSL #3          // Multiply by 8, this gives a bit offset                               \t\n\
        // Read -> Modify -> Write                                                                              \t\n\
        MOV     r12, #0xFF              // Mask (8 bits)                                                        \t\n\
        MOV     r12, r12, LSL r2        // Move mask into correct bit position                                  \t\n\
        MOV     r1, r1, LSL r2          // Also, move passed in priority value into correct bit position        \t\n\
        LDR     r3, [r0]                // Read current value of the Priority Level register (ICDIPR)           \t\n\
        BIC     r3, r3, r12             // Clear appropiate field                                               \t\n\
        ORR     r3, r3, r1              // Now OR in the priority value                                         \t\n\
        STR     r3, [r0]                // And store it back again  (ICDIPR)                                    \t\n\
                                                                                                                \t\n\
        //Join SMP                                                                                              \t\n\
        MOV     r0, #0x0                  // Move CPU ID into r0                                                \t\n\
        MOV     r1, #0xF                  // Move 0xF (represents all four ways) into r1                        \t\n\
                                                                                                                \t\n\
        //Secure SCU invalidate                                                                                 \t\n\
        AND     r0, r0, #0x03           // Mask off unused bits of CPU ID                                       \t\n\
        MOV     r0, r0, LSL #2          // Convert into bit offset (four bits per core)                         \t\n\
        AND     r1, r1, #0x0F           // Mask off unused bits of ways                                         \t\n\
        MOV     r1, r1, LSL r0          // Shift ways into the correct CPU field                                \t\n\
        MRC     p15, 4, r2, c15, c0, 0  // Read periph base address                                             \t\n\
        STR     r1, [r2, #0x0C]         // Write to SCU Invalidate All in Secure State                          \t\n\
                                                                                                                \t\n\
        //Now is joining                                                                                        \t\n\
        MRC     p15, 0, r0, c1, c0, 1   // Read ACTLR                                                           \t\n\
        ORR     r0, r0, #0x040          // Set bit 6                                                            \t\n\
        MCR     p15, 0, r0, c1, c0, 1   // Write ACTLR                                                          \t\n\
                                                                                                                \t\n\
        //Maintenance broadcast                                                                                 \t\n\
        MRC     p15, 0, r0, c1, c0, 1   // Read Aux Ctrl register                                               \t\n\
        ORR     r0, r0, #0x01           // Set the FW bit (bit 0)                                               \t\n\
        MCR     p15, 0, r0, c1, c0, 1   // Write Aux Ctrl register                                              \t\n\
                                                                                                                \t\n\
        //HOLDING                                                                                               \t\n\
        MOV     r2, #0x00                 // Clear r2                                                           \t\n\
        CPSIE   i                         // Enable interrupts                                                  \t\n\
    holding_pen:                                                                                                \t\n\
        CMP     r2, #0x0                  // r2 will be set to 0x1 by IRQ handler on receiving SGI              \t\n\
        WFIEQ                                                                                                   \t\n\
        BEQ     holding_pen                                                                                     \t\n\
        CPSID   i                         // IRQs not used in reset of example, so mask out interrupts          \t\n\
                                                                                                                \t\n\
    //skip:                                                                                                       \t\n\
        b _start                                                                                                \t\n\
                                                                                                                \t\n\
        ");

}


