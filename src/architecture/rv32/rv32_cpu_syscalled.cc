#include <architecture/rv32/rv32_cpu.h>

extern "C" { void _exec(void *); }

__BEGIN_SYS

void CPU::syscalled()
{

    if(Traits<Build>::MODE == Traits<Build>::KERNEL) {
        // Do the system call by calling _exec with the message pointed by a0
        ASM("        mv         a0,   a1                                      \n");
        ASM("        call    _exec                                            \n");

        CPU::sepc(CPU::sepc() + 0x4);

        CPU::sret();
    }
}

__END_SYS