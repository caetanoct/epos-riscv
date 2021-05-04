// EPOS Component Framework - Component Agent

#ifndef __agent_h
#define __agent_h

#include <process.h>
#include <real-time.h>
#include <machine/rtc.h>
#include <memory.h>
#include <time.h>
#include <synchronizer.h>

#include "message.h"

__BEGIN_SYS

class Agent: public Message
{
private:
    typedef void (Agent:: * Member)();

public:
    void exec() {
        if(id().type() != UTILITY_ID)
            db<Framework>(TRC) << ":=>" << *reinterpret_cast<Message *>(this) << endl;

        if(id().type() < LAST_TYPE_ID) // in-kernel services
            (this->*_handlers[id().type()])();
//        else { // out-of-kernel (i.e. Dom0 or server) services
//            db<Framework>(TRC) << "P3" << endl;
//                Message msg(*this); // copy message from user space to kernel
//                msg.id(Id(IPC_COMMUNICATOR_ID, id().unit()));
//                if(IPC::send(&msg)) { // 0 => no one listening
//                    Port<IPC> * comm = reinterpret_cast<Port<IPC> *>(IPC::observer(id().type())); // recall the Port<IPC> that got us here
//                    comm->receive(this); // copy from kernel to user
//                } else
//                    result(UNDEFINED);
//        }

        if(id().type() != UTILITY_ID)
            db<Framework>(TRC) << "<=:" << *reinterpret_cast<Message *>(this) << endl;
    }

private:
    void handle_thread();
    void handle_task();
    void handle_active();
    void handle_address_space();
    void handle_segment();
    void handle_mutex();
    void handle_semaphore();
    void handle_condition();
    void handle_clock();
    void handle_alarm();
    void handle_chronometer();
    void handle_utility();

private:
    static Member _handlers[LAST_TYPE_ID];
};


void Agent::handle_thread()
{
    Adapter<Thread> * thread = reinterpret_cast<Adapter<Thread> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE1: {
        int (*entry)();
        in(entry);
        id(Id(THREAD_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Thread>(Thread::Configuration(Thread::READY, Thread::NORMAL, WHITE, 0), entry))));
    } break;
    case DESTROY:
        delete thread;
        break;
    case SELF:
        id(Id(THREAD_ID, reinterpret_cast<Id::Unit_Id>(Adapter<Thread>::self())));
        break;
    case THREAD_PRIORITY:
        res = thread->priority();
        break;
    case THREAD_PRIORITY1: {
        int p;
        in(p);
        thread->priority(Thread::Criterion(p));
    } break;
    case THREAD_JOIN:
        res = thread->join();
        break;
    case THREAD_PASS:
        thread->pass();
        break;
    case THREAD_SUSPEND:
        thread->suspend();
        break;
    case THREAD_RESUME:
        thread->resume();
        break;
    case THREAD_YIELD:
        Thread::yield();
        break;
    case THREAD_WAIT_NEXT:
        Periodic_Thread::wait_next();
        break;
    case THREAD_EXIT: {
        int r;
        in(r);
        Thread::exit(r);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_task()
{
    Adapter<Task> * task = reinterpret_cast<Adapter<Task> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE3: {
        Segment * cs, * ds;
        int (*entry)();
        in(cs, ds, entry);
        id(Id(TASK_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Task>(cs, ds, entry))));
    } break;
    case DESTROY:
        delete task;
        break;
    case TASK_ADDRESS_SPACE:
        res = reinterpret_cast<int>(task->address_space());
        break;
    case TASK_CODE_SEGMENT:
        res = reinterpret_cast<int>(task->code_segment());
        break;
    case TASK_DATA_SEGMENT:
        res = reinterpret_cast<int>(task->data_segment());
        break;
    case TASK_CODE:
        res = task->code();
        break;
    case TASK_DATA:
        res = task->data();
        break;
    case TASK_MAIN:
        res = reinterpret_cast<int>(task->main());
        break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_active()
{
    result(UNDEFINED);
};


void Agent::handle_address_space()
{
    Adapter<Address_Space> * as = reinterpret_cast<Adapter<Address_Space> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE:
        id(Id(ADDRESS_SPACE_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Address_Space>())));
        break;
    case CREATE1:
        MMU::Page_Directory * pd;
        in(pd);
        id(Id(ADDRESS_SPACE_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Address_Space>(pd))));
        break;
    case DESTROY:
        delete as;
        break;
    case ADDRESS_SPACE_PD:
        res = as->pd();
        break;
    case ADDRESS_SPACE_ATTACH1: {
        Segment * seg;
        in(seg);
        res = as->attach(seg);
    } break;
    case ADDRESS_SPACE_ATTACH2: {
        Segment * seg;
        CPU::Log_Addr addr;
        in(seg, addr);
        res = as->attach(seg, addr);
    } break;
    case ADDRESS_SPACE_DETACH1: {
        Segment * seg;
        in(seg);
        as->detach(seg);
    } break;
    case ADDRESS_SPACE_DETACH2: {
        Segment * seg;
        CPU::Log_Addr addr;
        in(seg, addr);
        as->detach(seg, addr);
    } break;
    case ADDRESS_SPACE_PHYSICAL: {
        CPU::Log_Addr addr;
        in(addr);
        res = as->physical(addr);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_segment()
{
    Adapter<Segment> * seg = reinterpret_cast<Adapter<Segment> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE1: {
        unsigned int bytes;
        in(bytes);
        id(Id(SEGMENT_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Segment>(bytes))));
    } break;
    case CREATE2: { // *** indistinguishable ***
        unsigned int bytes;
    Segment::Flags flags;
    in(bytes, flags);
    id(Id(SEGMENT_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Segment>(bytes, WHITE, flags))));
    } break;
    case CREATE3: { // *** indistinguishable ***
        Segment::Phy_Addr phy_addr;
    unsigned int bytes;
    Segment::Flags flags;
    in(phy_addr, bytes, flags);
    id(Id(SEGMENT_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Segment>(phy_addr, bytes, flags))));
    } break;
    case DESTROY:
        delete seg;
        break;
    case SEGMENT_SIZE:
        res = seg->size();
        break;
    case SEGMENT_PHY_ADDRESS:
        res = seg->phy_address();
        break;
    case SEGMENT_RESIZE: {
        int amount;
        in(amount);
        res = seg->resize(amount);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_mutex()
{
    Adapter<Mutex> * mutex = reinterpret_cast<Adapter<Mutex> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE:
        id(Id(MUTEX_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Mutex>())));
        break;
    case DESTROY:
        delete mutex;
        break;
    case SYNCHRONIZER_LOCK:
        mutex->lock();
        break;
    case SYNCHRONIZER_UNLOCK:
        mutex->unlock();
        break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_semaphore()
{
    Adapter<Semaphore> * sem = reinterpret_cast<Adapter<Semaphore> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE:
        id(Id(SEMAPHORE_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Semaphore>())));
        break;
    case DESTROY:
        delete sem;
        break;
    case SYNCHRONIZER_P:
        sem->p();
        break;
    case SYNCHRONIZER_V:
        sem->v();
        break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_condition()
{
    Adapter<Condition> * cond = reinterpret_cast<Adapter<Condition> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE:
        id(Id(CONDITION_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Condition>())));
        break;
    case DESTROY:
        delete cond;
        break;
    case SYNCHRONIZER_WAIT:
        cond->wait();
        break;
    case SYNCHRONIZER_SIGNAL:
        cond->signal();
        break;
    case SYNCHRONIZER_BROADCAST:
        cond->broadcast();
        break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_clock()
{
    Adapter<Clock> * clock = reinterpret_cast<Adapter<Clock> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE:
        id(Id(CLOCK_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Clock>())));
        break;
    case CLOCK_RESOLUTION:
        res = clock->resolution();
        break;
    case CLOCK_NOW:
        res = clock->now();
        break;
    // case CLOCK_DATE:
    //     // res = reinterpret_cast<int>(clock->date());
    //     break;
    // case CLOCK_DATE1:
    //     Date & d;
    //     in(d);
    //     clock->date(d);
    //     break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_alarm()
{
    Adapter<Alarm> * alarm = reinterpret_cast<Adapter<Alarm> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE2: {
        Microsecond time;
        Handler * handler;
        in(time, handler);
        id(Id(ALARM_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Alarm>(time, handler))));
    } break;
    case CREATE3: {
        Microsecond time;
        Handler * handler;
        int times;
        in(time, handler, times);
        id(Id(ALARM_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Alarm>(time, handler, times))));
    } break;
    case DESTROY:
        delete alarm;
        break;
    case ALARM_GET_PERIOD:
        res = alarm->period();
    break;
    case ALARM_SET_PERIOD: {
        Microsecond p;
        in(p);
        alarm->period(p);
    } break;
    case ALARM_FREQUENCY:
        res = Adapter<Alarm>::alarm_frequency();
    break;
    case ALARM_DELAY: {
        Microsecond time;
        in(time);
        Adapter<Alarm>::delay(time);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_chronometer()
{
    Adapter<Alarm> * alarm = reinterpret_cast<Adapter<Alarm> *>(id().unit());
    Result res = 0;

    switch(method()) {
    case CREATE2: {
        Microsecond time;
        Handler * handler;
        in(time, handler);
        id(Id(ALARM_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Alarm>(time, handler))));
    } break;
    case CREATE3: {
        Microsecond time;
        Handler * handler;
        int times;
        in(time, handler, times);
        id(Id(ALARM_ID, reinterpret_cast<Id::Unit_Id>(new Adapter<Alarm>(time, handler, times))));
    } break;
    case DESTROY:
        delete alarm;
        break;
    case ALARM_GET_PERIOD:
        res = alarm->period();
    break;
    case ALARM_SET_PERIOD: {
        Microsecond p;
        in(p);
        alarm->period(p);
    } break;
    case ALARM_FREQUENCY:
        res = Adapter<Alarm>::alarm_frequency();
    break;
    case ALARM_DELAY: {
        Microsecond time;
        in(time);
        Adapter<Alarm>::delay(time);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};


void Agent::handle_utility()
{
    Result res = 0;

    switch(method()) {
    case PRINT: {
        const char * s;
        in(s);
        _print(s);
    } break;
    default:
        res = UNDEFINED;
    }

    result(res);
};

__END_SYS

#endif
