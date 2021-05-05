#include <architecture/rv32/rv32_cpu.h>
#include <machine/ic.h>

__BEGIN_SYS

void CPU::syscall(void * message)
{   
    ASM("mv %0, a0" :  "=r"(message)); 
    CPU::ecall();
}

__END_SYS
