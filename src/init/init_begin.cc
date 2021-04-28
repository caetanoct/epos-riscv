// EPOS First Thread Initializer

#include <system.h>
#include <machine.h>
#include <utility/string.h>

extern "C" char __bss_start;
extern "C" char _end;

__BEGIN_SYS

class Init_Begin
{
  public:
    Init_Begin() {
      if (Traits<Build>::MODE == Traits<Build>::KERNEL)
        memset(reinterpret_cast<void *>(__bss_start), 0, _end - __bss_start);

      Machine::pre_init(System::info());
    }
};

Init_Begin init_begin;

__END_SYS