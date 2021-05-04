#include <utility/spin.h>
#include <machine.h>
#include <process.h>

extern "C" {
    __USING_SYS;

    void _panic() { Machine::panic(); }
    void _exit(int s) { Thread::exit(s); for(;;); }
    void __exit() { Thread::exit(CPU::fr()); }  // must be handled by the Page Fault handler for user-level tasks
    void __cxa_pure_virtual() { db<void>(ERR) << "Pure Virtual method called!" << endl; }

    // Utility-related methods that differ from kernel and user space.
    // OStream
    void _print(const char * s) { Display::puts(s); }
}

#include <framework/main.h>
#include <framework/agent.h>

// Framework class attributes
__BEGIN_SYS

// IPC::Observed IPC::_observed;

Agent::Member Agent::_handlers[] = {&Agent::handle_thread,
                                    &Agent::handle_task,
                                    &Agent::handle_active,
                                    &Agent::handle_address_space,
                                    &Agent::handle_segment,
                                    &Agent::handle_mutex,
                                    &Agent::handle_semaphore,
                                    &Agent::handle_condition,
                                    &Agent::handle_clock,
                                    &Agent::handle_alarm,
                                    &Agent::handle_chronometer,
                                    &Agent::handle_utility
};

__END_SYS

__USING_SYS;
extern "C" { void _exec(void * m) { reinterpret_cast<Agent *>(m)->exec(); } }
