// EPOS Thread Component Declarations

#ifndef __process_h
#define __process_h

#include <architecture.h>
#include <machine.h>
#include <utility/queue.h>
#include <utility/list.h>
#include <utility/handler.h>
#include <scheduler.h>

extern "C" { void __exit(); }

__BEGIN_SYS

class Thread
{
    friend class Init_End;              // context->load()
    friend class Init_System;           // for init() on CPU != 0
    friend class Scheduler<Thread>;     // for link()
    friend class Synchronizer_Common;   // for lock() and sleep()
    friend class Alarm;                 // for lock()
    friend class System;                // for init()
    friend class IC;                    // for link() for priority ceiling
    friend class Task;

protected:
    static const bool preemptive = Traits<Thread>::Criterion::preemptive;
    static const bool reboot = Traits<System>::reboot;

    static const unsigned int QUANTUM = Traits<Thread>::QUANTUM;
    static const unsigned int STACK_SIZE = Traits<Application>::STACK_SIZE;
    static const unsigned int USER_STACK_SIZE = Traits<Application>::USER_STACK_SIZE;

    typedef CPU::Log_Addr Log_Addr;
    typedef CPU::Context Context;

public:
    // Thread State
    enum State {
        RUNNING,
        READY,
        SUSPENDED,
        WAITING,
        FINISHING
    };

    // Thread Scheduling Criterion
    typedef Traits<Thread>::Criterion Criterion;
    enum {
        HIGH    = Criterion::HIGH,
        NORMAL  = Criterion::NORMAL,
        LOW     = Criterion::LOW,
        MAIN    = Criterion::MAIN,
        IDLE    = Criterion::IDLE
    };

    // Thread Queue
    typedef Ordered_Queue<Thread, Criterion, Scheduler<Thread>::Element> Queue;

    // Thread Configuration
    struct Configuration {
        Configuration(
            const State & s = READY, 
            const Criterion & c = NORMAL, 
            const Color & a = WHITE,
            Task * t = 0,
            unsigned int ss = STACK_SIZE)
        : state(s), criterion(c), color(a), task(t), stack_size(ss) {}

        State state;
        Criterion criterion;
        Color color;
        Task * task;
        unsigned int stack_size;
    };


public:
    template<typename ... Tn>
    Thread(int (* entry)(Tn ...), Tn ... an);
    template<typename ... Tn>
    Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an);
    ~Thread();

    const volatile State & state() const { return _state; }

    const volatile Priority & priority() const { return _link.rank(); }
    void priority(const Priority & p);

    int join();
    void pass();
    void suspend();
    void resume();

    static Thread * volatile self() { return running(); }
    static void yield();
    static void exit(int status = 0);

    Task * task() const { return _task; }


protected:
    void constructor_prologue(unsigned int stack_size);
    void constructor_epilogue(const Log_Addr & entry, unsigned int stack_size);

    Criterion & criterion() { return const_cast<Criterion &>(_link.rank()); }
    Queue::Element * link() { return &_link; }

    static Thread * volatile running() { return _scheduler.chosen(); }

    static void lock() { CPU::int_disable(); }
    static void unlock() { CPU::int_enable(); }
    static bool locked() { return CPU::int_disabled(); }

    static void sleep(Queue * q);
    static void wakeup(Queue * q);
    static void wakeup_all(Queue * q);

    static void reschedule();
    static void time_slicer(IC::Interrupt_Id interrupt);

    static void dispatch(Thread * prev, Thread * next, bool charge = true);

    static int idle();

private:
    static void init();

protected:
    char * _stack;
    Context * volatile _context;
    volatile State _state;
    Queue * _waiting;
    Thread * volatile _joining;
    Queue::Element _link;

    Task * _task;
    Segment * _ustack;

    static volatile unsigned int _thread_count;
    static Scheduler_Timer * _timer;
    static Scheduler<Thread> _scheduler;
};

class Task {
    friend class Init_End;
    friend class System;
    friend class Thread;

private:
    typedef CPU::Log_Addr Log_Addr;
    
    typedef Thread::Queue Queue;

protected: 
    template<typename ... Tn>
    Task(Address_Space * as, Segment * cs, Segment * ds, int (* entry)(Tn ...), 
        const Log_Addr & code, const Log_Addr & data, Tn ... an) : 
        _as(as), _cs(cs), _ds(ds), _entry(entry),
        _code(code),
        _data(data)
    {
        db<Task>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds <<  ",code=" << _code << ",data=" << _data << ") => " << this << endl;

        _current = this;
        activate();

        _main = new (SYSTEM) Thread(
            Thread::Configuration(
                Thread::RUNNING, Thread::MAIN, WHITE, this, 0), 
                entry, 
                an ...
        );

    }

public: 
    template<typename ... Tn>
    Task(
        Segment * cs, Segment * ds, 
        int (* entry)(Tn ...), Tn ... an)
    : _as (new (SYSTEM) Address_Space), _cs(cs), _ds(ds), _entry(entry), _code(_as->attach(_cs)), _data(_as->attach(_ds)) {
        db<Task>(TRC) << "Task(as=" << _as << ",cs=" << _cs << ",ds=" << _ds << ",entry=" << _entry << ",code=" << _code << ",data=" << _data << ") => " << this << endl;

        _main = new (SYSTEM) Thread(Thread::Configuration(Thread::READY, Thread::MAIN, WHITE, this, 0), entry, an ...);
    }


    ~Task();
    
    Address_Space * address_space() const { return _as; }

    Segment * code_segment() const { return _cs; }
    Segment * data_segment() const { return _ds; }
    
    Log_Addr code() const { return _code; }
    Log_Addr data() const { return _data; }

    Thread * main() const { return _main; }

    static Task * self() { return _current; }
private:
    void insert(Thread *thread)
    {
        _threads.insert(new (SYSTEM) Queue::Element(thread));
    }

    void remove(Thread *thread)
    {
        Queue::Element *el = _threads.remove(thread);

        if (el)
        {
            delete el;
        }
    }

    void remove()
    {
        Queue::Element *el = _threads.remove();

        if (el)
        {
            delete el;
        }
    }

    void activate() const
    {
        this->_as->activate();
    }

    static volatile Task *current() { return _current; }
    static volatile void current(Task *t) { _current = t; }

    static void init();

private:
    Address_Space * _as;
    Segment * _cs;
    Segment * _ds;
    Log_Addr _entry;
    Log_Addr _code;
    Log_Addr _data;

    Queue _threads;

    Thread * _main;

    static Task * volatile _current;
};


template<typename ... Tn>
inline Thread::Thread(int (* entry)(Tn ...), Tn ... an)
: _task(Task::self()), _ustack(0), _state(READY), _waiting(0), _joining(0), _link(this, NORMAL)
{
    constructor_prologue(STACK_SIZE);
    _context = CPU::init_stack(0, _stack + STACK_SIZE, &__exit, entry, an ...);
    constructor_epilogue(entry, STACK_SIZE);
}

template<typename ... Tn>
inline Thread::Thread(const Configuration & conf, int (* entry)(Tn ...), Tn ... an)
: _task(conf.task ? conf.task : Task::self()), _state(conf.state), _waiting(0), _joining(0), _link(this, conf.criterion)
{
    if (Traits<System>::multitask && !conf.stack_size) {
        constructor_prologue(STACK_SIZE);
        _ustack = new (SYSTEM) Segment(USER_STACK_SIZE);

        Log_Addr ustack = Task::self()->address_space()->attach(_ustack);

        Log_Addr user_sp = ustack + USER_STACK_SIZE;
        if (conf.criterion == MAIN)
            user_sp = CPU::init_user_stack(user_sp, 0, an ...);
        else
            user_sp = CPU::init_user_stack(user_sp, &__exit, an ...);

        Task::self()->address_space()->detach(_ustack, ustack);
        ustack = _task->address_space()->attach(_ustack);

        user_sp = ustack + USER_STACK_SIZE - user_sp;

        _context = CPU::init_stack(user_sp, _stack + STACK_SIZE, &__exit, entry, an ...);
    } else {
        constructor_prologue(conf.stack_size);
        _ustack = 0;
        _context = CPU::init_stack(0, _stack + conf.stack_size, &__exit, entry, an ...);
    }

    constructor_epilogue(entry, STACK_SIZE);
}


// A Java-like Active Object
class Active: public Thread
{
public:
    Active(): Thread(Configuration(Thread::SUSPENDED), &entry, this) {}
    virtual ~Active() {}

    virtual int run() = 0;

    void start() { resume(); }

private:
    static int entry(Active * runnable) { return runnable->run(); }
};


// An event handler that triggers a thread (see handler.h)
class Thread_Handler : public Handler
{
public:
    Thread_Handler(Thread * h) : _handler(h) {}
    ~Thread_Handler() {}

    void operator()() { _handler->resume(); }

private:
    Thread * _handler;
};



__END_SYS

#endif
