#include <architecture/rv32/rv32_cpu.h>

extern "C" { void _exec(void *); }

__BEGIN_SYS

void CPU::syscalled(void * message)
{
    _exec(message);
}

__END_SYS