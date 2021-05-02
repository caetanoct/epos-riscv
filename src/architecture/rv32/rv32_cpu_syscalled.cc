#include <architecture/rv32/rv32_cpu.h>

extern "C" { void _exec(void *); }

__BEGIN_SYS

void CPU::syscalled(void * msg)
{

    if(Traits<Build>::MODE == Traits<Build>::KERNEL) {
        // Do the system call by calling _exec with the message pointed by a0
        ASM("        call    _exec                                           \n");

        CPU::sepc(CPU::sepc() + 0x4);

        ASM("        sret                                                    \n");
    }
}

__END_SYS
