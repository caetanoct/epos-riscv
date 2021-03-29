// EPOS RISC-V sifive SETUP

#include <system/config.h>
#include <architecture/cpu.h>
#include <architecture/mmu.h>
#include <machine/timer.h>
#include <machine/ic.h>

using namespace EPOS::S;
typedef unsigned int Reg;

extern "C" 
{
    void _setup() __attribute__ ((used, naked, section(".init")));
    void _mmode_forward() __attribute__((naked));
    void _int_entry();
    void _start();
    void _wait() 
    { 
        CPU::halt();
        _start();
    }
}

// Interrupt Vector Table
void _setup()
{
    ASM("\t\n\
        j       .reset                                                          \t\n\
                                                                                \t\n\
.reset:                                                                         \t\n\
        # Disable interrupts                                                    \t\n\
        csrs    mstatus, 1 << 3                                                 \t\n\
                                                                                \t\n\
        # Disable paging                                                        \t\n\
        csrw    sptbr, zero                                                     \t\n\
                                                                                \t\n\
        # Put CLINT in direct mode (mtvec.mode = 0) and set mtvec to _int_entry \t\n\
        la      t0, _int_entry                                                  \t\n\
        andi    t0, t0, 0xfffffffe  # mtvec.mode = 0                            \t\n\
        csrw    mtvec, t0                                                       \t\n\
                                                                                \t\n\
        # Get the hart's id                                                     \t\n\
        csrr    a0, mhartid                                                     \t\n\
                                                                                \t\n\
        # Set a 16KB stack for each hart (#0 at __boot_stack__)                 \t\n\
        la      sp, __boot_stack__                                              \t\n\
        li      t0, 1                                                           \t\n\
        slli    t0, t0, 14                                                      \t\n\
        mul     t0, t0, a0                                                      \t\n\
        sub     sp, sp, t0                                                      \t\n\
                                                                                \t\n\
        # Non-bootstrapping harts wait for an IPI                               \t\n\
        bnez    a0, .secondary                                                  \t\n\
                                                                                \t\n\
        # Set mstatus to machine mode with interrupts disabled                  \t\n\
        # 0b11 << 11: Machine's previous protection mode is 3 (MPP=3)           \t\n\
        #    1 <<  7: Machine's previous interrupt-enable bit is 1 (MPIE=1)     \t\n\
        li      t0, (0b11 << 11) | (1 << 7)                                     \t\n\
        csrw    mstatus, t0                                                     \t\n\
                                                                                \t\n\
        # Set mepc to `_start` (will be used by mret)                           \t\n\
        la      t0, _start                                                      \t\n\
        csrw    mepc, t0                                                        \t\n\
                                                                                \t\n\
        # Go to _start and update mstatus accordingly by returning to mepc      \t\n\
        mret                                                                    \t\n\
                                                                                \t\n\
.secondary:                                                                     \t\n\
        # Set mstatus to machine mode with interrupts enabled                   \t\n\
        # 0b11 << 11: Machine's previous protection mode is 3 (MPP=3)           \t\n\
        #    1 <<  7: Machine's previous interrupt-enable bit is 1 (MPIE=1)     \t\n\
        #    1 <<  3: Machine's interrupt-enable bit is 1 (MIE=1)               \t\n\
        li      t0, (0b11 << 11) | (1 << 7) | (1 << 3)                          \t\n\
        csrw    mstatus, t0                                                     \t\n\
                                                                                \t\n\
        # Enable software interrupts so hart #0 can latter wake up this hart    \t\n\
        li      t0, (1 << 3) | (1 << 7) | (1 << 11)                             \t\n\
        csrw    mie, t0                                                         \t\n\
                                                                                \t\n\
        # Set mepc to `_wait` (will be used by mret)                            \t\n\
        la  t0, .wait                                                           \t\n\
        csrw mepc, t0                                                           \t\n\
                                                                                \t\n\
        # Go to _wait and update mstatus accordingly by returning to mepc       \t\n\
        mret                                                                    \t\n\
                                                                                \t\n\
.wait:                                                                          \t\n\
        wfi                                                                     \t\n\
        j _start                                                                \t\n\
        ");
}

class Setup_SifiveE {
private:
    typedef CPU::Reg Reg;
    typedef MMU::RV32_Flags RV32_Flags;
    typedef MMU::Page_Table Page_Table;
    typedef MMU::Page_Directory Page_Directory;
    typedef MMU::PT_Entry PT_Entry;

public:
    static void init() { setup_machine_environment(); }
    static void setup_machine_environment();
    static void setup_supervisor_environment();
    static void build_page_tables();
};

__BEGIN_SYS

class Setup_SifiveE {
private:
    typedef CPU::Reg Reg;
    typedef MMU::RV32_Flags RV32_Flags;
    typedef MMU::Page_Table Page_Table;
    typedef MMU::Page_Directory Page_Directory;
    typedef MMU::PT_Entry PT_Entry;

public:
    static void init() { setup_machine_environment(); }
    static void setup_machine_environment();
    static void setup_supervisor_environment();
    static void build_page_tables();
};

void Setup_SifiveE::build_page_tables() 
{
    Reg page_tables = Traits<Machine>::PAGE_TABLES; // address of the page table root
    MMU::_master = new ( (void *) page_tables ) Page_Directory();

    for(int i = 0; i < 1024; i++) { 
        PT_Entry * pte = (((PT_Entry *)MMU::_master) + i);
        * pte = ((page_tables >> 12) << 10);
        * pte += ((i+1) << 10);
        * pte |= MMU::RV32_Flags::VALID;    
    }

    for(int i = 0; i < 1024; i++)
    {
        Page_Table * pt = new ( (void *)(page_tables + 4*1024*(i+1))  ) Page_Table();
        pt->remap(RV32_Flags::SYS);
    }
}

void Setup_SifiveE::setup_supervisor_environment() 
{
    CPU::satp((0x1 << 31) | (Traits<Machine>::PAGE_TABLES >> 12));
    ASM("sret");
}

void Setup_SifiveE::setup_machine_environment()
{
    CPU::mmode_int_disable();
    Reg core = CPU::mhartid();
    CPU::tp(core);
    // set stack for each core
    CPU::sp(Traits<Machine>::BOOT_STACK - Traits<Machine>::STACK_SIZE * core);
    CPU::mstatus_write(CPU::MPP_S | CPU::MPIE);
    CPU::mepc((unsigned)&setup_supervisor_environment);
    CPU::mtvec((unsigned)&_mmode_forward & 0xfffffffc);
    CPU::mie_write(CPU::MTI);

    build_page_tables();
    CPU::satp_write(0); // paging off
    
    // forward everything
    CPU::sstatus_write( CPU::SIE | CPU::SPIE | CPU::SPP_S );
    CPU::sie_write( CPU::SSI | CPU::STI | CPU::SEI );
    CPU::stvec_write((unsigned)&_int_entry & 0xfffffffc);
    CPU::sepc_write((unsigned)&_start);
    
    // delegate everything
    CPU::mideleg_write(CPU::SSI | CPU::STI | CPU::SEI);
    CPU::medeleg_write(0xffff);
    
    ASM("mret");
}

__END_SYS

void _setup() { Setup_SifiveE::init(); }