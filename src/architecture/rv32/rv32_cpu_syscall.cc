#include <architecture/rv32/rv32_cpu.h>
#include <machine/ic.h>

__BEGIN_SYS

void CPU::syscall(void * msg)
{   
    ASM("ecall");
}

__END_SYS
