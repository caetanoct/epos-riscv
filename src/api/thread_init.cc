// EPOS Thread Initialization

#include <machine/timer.h>
#include <machine/ic.h>
#include <system.h>
#include <process.h>
#include <memory.h>
#include <utility/elf.h>

__BEGIN_SYS

extern "C" { void __epos_app_entry(); }

void Thread::init()
{
    db<Init, Thread>(TRC) << "Thread::init()" << endl;

    typedef int (Main)(int argc, char * argv[]);

    System_Info * si = System::info();
    Main * main;

    if(Traits<System>::multitask)
        main = reinterpret_cast<Main *>(si->lm.app_entry);
    else
        // If EPOS is a library, then adjust the application entry point to __epos_app_entry, which will directly call main().
        // In this case, _init will have already been called, before Init_Application to construct MAIN's global objects.
        main = reinterpret_cast<Main *>(__epos_app_entry);

    Criterion::init();

    if(Traits<System>::multitask) {

        int app_extra_size = static_cast<int>(si->lm.app_extra_size);
        char ** app_extra = reinterpret_cast<char **>(si->lm.app_extra);

        db<Init>(TRC) << "extra size " << si->lm.app_extra_size << "extra " << app_extra << endl;

        new (SYSTEM) Task(
            new (SYSTEM) Address_Space(MMU::current()),
            new (SYSTEM) Segment(Log_Addr(si->lm.app_code), si->lm.app_code_size, MMU::Page_Flags::APP, true),
            new (SYSTEM) Segment(Log_Addr(si->lm.app_data), si->lm.app_data_size, MMU::Page_Flags::APP, true),
            main,
            Log_Addr(Memory_Map::APP_CODE), Log_Addr(Memory_Map::APP_DATA),
            app_extra_size, app_extra);

        if(si->lm.has_ext) {
            db<Init>(INF) << "Thread::init: additional data from mkbi at "  << reinterpret_cast<void *>(si->lm.app_extra) << ":" << si->lm.app_extra_size << endl;
            
            ELF *extra_elf = reinterpret_cast<ELF *>(si->lm.app_extra + sizeof(int));
            db<Setup>(TRC) << "App_elf " << extra_elf << endl;
            
            if (extra_elf->valid()) {
                unsigned int extra_app_data = ~0U;
                unsigned int extra_app_data_size = 0;
                
                unsigned int extra_app_code = extra_elf->segment_address(0);
                unsigned int extra_app_code_size = extra_elf->segment_size(0);

                for (int i = 1; i < extra_elf->segments(); i++) {
                    if (extra_elf->segment_type(i) != PT_LOAD)
                        continue;
                    if (extra_elf->segment_address(i) < extra_app_data)
                        extra_app_data = extra_elf->segment_address(i);

                    extra_app_data_size += extra_elf->segment_size(i);
                }

                extra_app_data_size = MMU::align_page(extra_app_data_size);
                extra_app_data_size += MMU::align_page(Traits<Application>::STACK_SIZE);
                extra_app_data_size += MMU::align_page(Traits<Application>::HEAP_SIZE);

                new (SYSTEM) Task(
                    new (SYSTEM) Address_Space(MMU::current()),
                    new (SYSTEM) Segment(Log_Addr(extra_app_code), extra_app_code_size, Segment::Flags::APP),
                    new (SYSTEM) Segment(Log_Addr(extra_app_data), extra_app_data_size, Segment::Flags::APP),
                    reinterpret_cast<Main *>(extra_elf->entry()),
                    Log_Addr(extra_app_code), Log_Addr(extra_app_data),
                    static_cast<int>(0), reinterpret_cast<char **>(0)
                );
                
            }
        }
    } else {
        // If EPOS is a library, then adjust the application entry point to __epos_app_entry,
        // which will directly call main(). In this case, _init will already have been called,
        // before Init_Application to construct MAIN's global objects.
        new (SYSTEM) Thread(Thread::Configuration(Thread::RUNNING, Thread::MAIN), reinterpret_cast<int (*)()>(main));
    }

    // Idle thread creation does not cause rescheduling (see Thread::constructor_epilogue)
    new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::IDLE), &Thread::idle);

    // The installation of the scheduler timer handler does not need to be done after the
    // creation of threads, since the constructor won't call reschedule() which won't call
    // dispatch that could call timer->reset()
    // Letting reschedule() happen during thread creation is also harmless, since MAIN is
    // created first and dispatch won't replace it nor by itself neither by IDLE (which
    // has a lower priority)
    if(Criterion::timed)
        _timer = new (SYSTEM) Scheduler_Timer(QUANTUM, time_slicer);

    // No more interrupts until we reach init_end
    CPU::int_disable();

    // Transition from CPU-based locking to thread-based locking
    This_Thread::not_booting();
}

__END_SYS
