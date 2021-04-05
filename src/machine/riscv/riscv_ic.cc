// EPOS RISC-V IC Mediator Implementation

#include <machine/machine.h>
#include <machine/ic.h>
#include <architecture/cpu.h>
#include <machine/timer.h>

extern "C" { void _int_entry() __attribute__ ((alias("_ZN4EPOS1S2IC5entryEv"))); }

__BEGIN_SYS

typedef unsigned long Reg;

// Class attributes
IC::Interrupt_Handler IC::_int_vector[IC::INTS];

// Class methods
void IC::entry()
{
    // Handle interrupts in machine mode
    ASM("        .align 4                                               \n");
    
    ASM("        addi        sp,     sp,   -136                         \n");
    ASM("        sw          x1,   4(sp)                                \n");
    ASM("        sw          x2,   8(sp)                                \n");
    ASM("        sw          x3,  12(sp)                                \n");
    ASM("        sw          x4,  16(sp)                                \n");
    ASM("        sw          x5,  20(sp)                                \n");
    ASM("        sw          x6,  24(sp)                                \n");
    ASM("        sw          x7,  28(sp)                                \n");
    ASM("        sw          x8,  32(sp)                                \n");
    ASM("        sw          x9,  36(sp)                                \n");
    ASM("        sw         x10,  40(sp)                                \n");
    ASM("        sw         x11,  44(sp)                                \n");
    ASM("        sw         x12,  48(sp)                                \n");
    ASM("        sw         x13,  52(sp)                                \n");
    ASM("        sw         x14,  56(sp)                                \n");
    ASM("        sw         x15,  60(sp)                                \n");
    ASM("        sw         x16,  64(sp)                                \n");
    ASM("        sw         x17,  68(sp)                                \n");
    ASM("        sw         x18,  72(sp)                                \n");
    ASM("        sw         x19,  76(sp)                                \n");
    ASM("        sw         x20,  80(sp)                                \n");
    ASM("        sw         x21,  84(sp)                                \n");
    ASM("        sw         x22,  88(sp)                                \n");
    ASM("        sw         x23,  92(sp)                                \n");
    ASM("        sw         x24,  96(sp)                                \n");
    ASM("        sw         x25, 100(sp)                                \n");
    ASM("        sw         x26, 104(sp)                                \n");
    ASM("        sw         x27, 108(sp)                                \n");
    ASM("        sw         x28, 112(sp)                                \n");
    ASM("        sw         x29, 116(sp)                                \n");
    ASM("        sw         x30, 120(sp)                                \n");
    ASM("        sw         x31, 124(sp)                                \n");
    ASM("        csrr       x31, sstatus                                \n");
    ASM("        sw         x31, 128(sp)                                \n");
    ASM("        csrr       x31, sepc                                   \n");
    ASM("        sw         x31, 132(sp)                                \n");

    Reg id = CPU::mcause();
    if(id & CLINT::INTERRUPT) {
        if ((id & IC::INT_MASK) == CLINT::IRQ_MAC_SOFT) {
            IC::ipi_eoi(id & IC::INT_MASK);
        }
        if ((id & IC::INT_MASK) == CLINT::IRQ_MAC_TIMER) {
            Timer::reset();
            CPU::sie(CPU::STI);
        }
        Reg interrupt_id = 1 << ((id & IC::INT_MASK) - 2);
        if (CPU::int_enabled() && (CPU::sie() & (interrupt_id))) {
            CPU::mip(interrupt_id); // Mandar pro supervisor
        }
    }

    ASM("        la          ra, .restore                               \n");
    ASM("        j          %0                                          \n" : : "i"(&dispatch));
    ASM(".restore:                                                      \n");
    ASM("        lw          x1,   4(sp)                                \n");
    ASM("        lw          x2,   8(sp)                                \n");
    ASM("        lw          x3,  12(sp)                                \n");
    ASM("        lw          x4,  16(sp)                                \n");
    ASM("        lw          x5,  20(sp)                                \n");
    ASM("        lw          x6,  24(sp)                                \n");
    ASM("        lw          x7,  28(sp)                                \n");
    ASM("        lw          x8,  32(sp)                                \n");
    ASM("        lw          x9,  36(sp)                                \n");
    ASM("        lw         x10,  40(sp)                                \n");
    ASM("        lw         x11,  44(sp)                                \n");
    ASM("        lw         x12,  48(sp)                                \n");
    ASM("        lw         x13,  52(sp)                                \n");
    ASM("        lw         x14,  56(sp)                                \n");
    ASM("        lw         x15,  60(sp)                                \n");
    ASM("        lw         x16,  64(sp)                                \n");
    ASM("        lw         x17,  68(sp)                                \n");
    ASM("        lw         x18,  72(sp)                                \n");
    ASM("        lw         x19,  76(sp)                                \n");
    ASM("        lw         x20,  80(sp)                                \n");
    ASM("        lw         x21,  84(sp)                                \n");
    ASM("        lw         x22,  88(sp)                                \n");
    ASM("        lw         x23,  92(sp)                                \n");
    ASM("        lw         x24,  96(sp)                                \n");
    ASM("        lw         x25, 100(sp)                                \n");
    ASM("        lw         x26, 104(sp)                                \n");
    ASM("        lw         x27, 108(sp)                                \n");
    ASM("        lw         x28, 112(sp)                                \n");
    ASM("        lw         x29, 116(sp)                                \n");
    ASM("        lw         x30, 120(sp)                                \n");
    ASM("        lw         x31, 128(sp)                                \n");
    ASM("        csrw   sstatus, x31                                    \n");
    ASM("        lw         x31, 132(sp)                                \n");
    ASM("        csrw      sepc, x31                                    \n");
    ASM("        lw         x31, 124(sp)                                \n");
    ASM("        addi        sp, sp,    136                             \n");
    ASM("        sret                                                   \n");
}

void IC::dispatch()
{
    Interrupt_Id id = int_id();

    if((id != INT_SYS_TIMER) || Traits<IC>::hysterically_debugged)
        db<IC>(TRC) << "IC::dispatch(i=" << id << ")" << endl;

    // IPIs must be acknowledged before calling the ISR, because in RISC-V, set bits will keep on triggering interrupts until they are cleared
    if(id == INT_RESCHEDULER)
        IC::ipi_eoi(id);

    _int_vector[id](id);
}

void IC::int_not(Interrupt_Id id)
{
    db<IC>(WRN) << "IC::int_not(i=" << id << ")";
    if(Traits<Build>::hysterically_debugged)
        db<IC>(ERR) << endl;
    else
        db<IC>(WRN) << endl;
}

void IC::exception(Interrupt_Id id)
{
    CPU::Reg mstatus = CPU::mstatus();
    CPU::Reg mcause = CPU::mcause();
    CPU::Reg mhartid = CPU::id();
    CPU::Reg mepc;
    ASM("csrr %0, mepc" : "=r"(mepc) : :);
    CPU::Reg sepc;
    ASM("csrr %0, sepc" : "=r"(sepc) : :);
    CPU::Reg mtval;
    ASM("csrr %0, mtval" : "=r"(mtval) : :);

    db<IC>(WRN) << "IC::Exception(" << id << ") => {" << hex << "mstatus=" << mstatus << ",mcause=" << mcause << ",mhartid=" << mhartid << ",mepc=" << hex << mepc << ",sepc=" << sepc << ",mtval=" << mtval << "}" << dec;

    switch(id) {
        case 0: // unaligned Instruction
        case 1: // instruction access failure
            db<IC>(WRN) << " => prefetch abort";
            break;
        case 2: // illegal instruction
            db<IC>(WRN) << " => illegal instruction";
            break;
        case 3: // Break Point
            db<IC>(WRN) << " => break point";
            break;
        case 4: // unaligned load address
        case 5: // load access failure
        case 6: // unaligned store address
        case 7: // store access failure
            db<IC>(WRN) << " => unaligned data";
            break;
        case 8: // user-mode environment call
        case 9: // supervisor-mode environment call
        case 10: // reserved... not described
        case 11: // machine-mode environment call
            db<IC>(WRN) << " => reserved";
            break;
        case 12: // Instruction Page Table failure
        case 13: // Load Page Table failure
        case 14: // reserved... not described
        case 15: // Store Page Table failure
            db<IC>(WRN) << " => data abort";
            break;
        default:
            int_not(id);
            break;
    }

    if(Traits<Build>::hysterically_debugged)
        db<IC>(ERR) << endl;
    else
        db<IC>(WRN) << endl;
}

__END_SYS
