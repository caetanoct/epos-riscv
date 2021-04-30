// EPOS Task Implementation

#include <process.h>

__BEGIN_SYS

// Methods
Task::~Task()
{
    db<Task>(TRC) << "~Task(this=" << this << ")" << endl;

    while(!_threads->empty())
        delete _threads->remove()->object();

    delete _as;
}

void Task::set_current(volatile Task * task) {
    Task::_current = task;
    task->_as->activate();
} 

__END_SYS
