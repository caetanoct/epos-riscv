// EPOS Thread Initialization

#include <machine/timer.h>
#include <machine/ic.h>
#include <system.h>
#include <process.h>
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

        char ** app_extra = reinterpret_cast<char **>(si->lm.app_extra);

        Segment * data_segment = new (SYSTEM) Segment(Log_Addr(si->lm.app_data), si->lm.app_data_size, MMU::Page_Flags::APP, true);

        new (SYSTEM) Task(new (SYSTEM) Address_Space(MMU::current()),
                          new (SYSTEM) Segment(Log_Addr(si->lm.app_code), si->lm.app_code_size, MMU::Page_Flags::APP, true),
                          data_segment,
                          main,
                          Log_Addr(Memory_Map::APP_CODE), Log_Addr(Memory_Map::APP_DATA),
                          static_cast<int>(si->lm.app_extra_size), app_extra);
        // Problema na leitura do elf segment zerado
        if(si->lm.has_ext) {
            db<Init>(INF) << "Thread::init: additional data from mkbi at "  << reinterpret_cast<void *>(si->lm.app_extra) << ":" << si->lm.app_extra_size << endl;
            for (unsigned int i = 1; i < si->bm.n_apps; i++) {
                db<Task, Init>(TRC) << "Task load extra " << i << endl;
                ELF *extra_elf = reinterpret_cast<ELF *>(&app_extra[si->bm.application_offset[i]]);
                if (Traits<Setup>::hysterically_debugged)
                {
                    db<Task, Init>(INF) << "Extra::app_elf: " << (void *)extra_elf << endl;
                    db<Task, Init>(INF) << "Extra::app_elf: " << MMU::Translation(extra_elf) << endl;
                    // db<Task, Init>(INF) << "Extra::app_elf[0]: " << MMU::Translation(extra_elf->segment_address(0)) << endl;
                    // db<Task, Init>(INF) << "Extra::app_elf[0].size: " << extra_elf->segment_size(0) << endl;
                }
                if (extra_elf->load_segment(0) < 0)
                {
                    db<Task, Init>(ERR) << "Application code segment was corrupted during INIT!" << endl;
                    Machine::panic();
                }
                for (int i = 1; i < extra_elf->segments(); i++)
                {
                    if (extra_elf->load_segment(i) < 0)
                    {
                        db<Task, Init>(ERR) << "Application data segment was corrupted during INIT!" << endl;
                        Machine::panic();
                    }
                }

                new (SYSTEM) Task(
                    new (SYSTEM) Segment(Log_Addr(extra_elf->segment_address(0)), extra_elf->segment_size(0), MMU::Page_Flags::APP, WHITE),
                    data_segment,
                    reinterpret_cast<Main *>(extra_elf->entry()),
                    static_cast<int>(MMU::align_page(si->lm.app_extra_size)),
                    app_extra);
                    

                // main_task->address_space()->attach(t->code_segment());
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
