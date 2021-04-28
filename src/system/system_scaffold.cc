// EPOS System Scaffold and System Component Implementation

#include <utility/ostream.h>
#include <utility/heap.h>
#include <machine.h>
#include <memory.h>
#include <process.h>
#include <system.h>

__BEGIN_SYS

OStream kout;
OStream kerr;

// System class attributes
System_Info * System::_si = reinterpret_cast<System_Info *>(Memory_Map::SYS_INFO);
char System::_preheap[];
Segment * System::_heap_segment;
Heap * System::_heap;

extern "C" {
    __USING_SYS;

    void _panic() { Machine::panic(); }
    void _exit(int s) { Thread::exit(s); for(;;); }
    void __exit() { Thread::exit(CPU::fr()); }
    void __cxa_pure_virtual() { db<void>(ERR) << "Pure Virtual method called!" << endl; }

    void _print(const char * s) { Display::puts(s); }
}

__END_SYS
