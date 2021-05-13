// EPOS RISC-V 32 System Call Entry Implementation

#include <architecture/rv32/rv32_cpu.h>

extern "C" { void _exec(void *); }

__BEGIN_SYS

void CPU::syscalled(void * message) {
    // We get here when an APP triggers INT_SYSCALL (i.e. ecall)
    if(Traits<Build>::MODE == Traits<Build>::KERNEL) {
        _exec(reinterpret_cast<void *>(message)); 
    }
}
__END_SYS
