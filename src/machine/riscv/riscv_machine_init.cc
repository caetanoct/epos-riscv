// EPOS RISC V Initialization

#include <machine.h>

__BEGIN_SYS

void Machine::smp_barrier_init(unsigned int n_cpus)
{
    IC::int_vector(IC::INT_RESCHEDULER, IC::ipi_eoi);
    Machine::delay(100000);
    for(unsigned int i = 1; i < n_cpus; i++) {
        IC::ipi(i, IC::INT_RESCHEDULER);
    }
}

void Machine::pre_init(System_Info * si) 
{
    if (Traits<System>::multitask)
        CLINT::stvec(CLINT::DIRECT, &IC::entry);

    if (Traits<System>::multicore && (CPU::id() == 0))
        smp_barrier_init(Traits<Build>::CPUS);

    if (CPU::id() == 0)
        Display::init();

    db<Init, Machine>(TRC) << "Machine::pre_init()" << endl;
}

void Machine::init()
{
    db<Init, Machine>(TRC) << "Machine::init()" << endl;

    if(Traits<IC>::enabled)
        IC::init();

    if(Traits<Timer>::enabled)
        Timer::init();
}

__END_SYS
