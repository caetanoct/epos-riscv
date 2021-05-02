#include <utility/ostream.h>
#include <time.h>
#include <real-time.h>

using namespace EPOS;

OStream cout;
Chronometer chrono;
Periodic_Thread * thread_a;
Periodic_Thread * thread_b;
Periodic_Thread * thread_c;


int func_a();
int func_b();
int func_c();
long max(unsigned int a, unsigned int b, unsigned int c);

const unsigned int iterations = 100;
const unsigned int period_a = 100; // ms
const unsigned int period_b = 80; // ms
const unsigned int period_c = 60; // ms
const unsigned int wcet_a = 50; // ms
const unsigned int wcet_b = 20; // ms
const unsigned int wcet_c = 10; // ms

void test_allocator();
void test_scheduler();

int main()
{
    test_allocator();
    test_scheduler();
    return 0;
}

void test_allocator() {
    cout << "Test do alocador de memoria" << endl;

    int * ptr_1 = new int[15];
    int * ptr_2 = new int[120];
    int * ptr_3 = new int[50];

    cout << "ptr-1 addr: " << ptr_1 << endl;
    cout << "ptr-2 addr: " << ptr_2 << endl;
    cout << "ptr-3 addr: " << ptr_3 << endl;

    delete ptr_1;
    delete ptr_2;
    delete ptr_3;

}


long max(unsigned int a, unsigned int b, unsigned int c) { return ((a >= b) && (a >= c)) ? a : ((b >= a) && (b >= c) ? b : c); }


inline void exec(char c, unsigned int time = 0) // in miliseconds
{
    // Delay was not used here to prevent scheduling interference due to blocking
    Microsecond elapsed = chrono.read() / 1000;

    cout << "\n" << elapsed << "\t" << c
         << "\t[p(A)=" << thread_a->priority()
         << ", p(B)=" << thread_b->priority()
         << ", p(C)=" << thread_c->priority() << "]";

    if(time) {
        for(Microsecond end = elapsed  time, last = end; end > elapsed; elapsed = chrono.read() / 1000)
            if(last != elapsed) {
                cout << "\n" << elapsed << "\t" << c
                    << "\t[p(A)=" << thread_a->priority()
                    << ", p(B)=" << thread_b->priority()
                    << ", p(C)=" << thread_c->priority() << "]";
                last = elapsed;
            }
    }
}


void test_scheduler()
{
    cout << "Teste do escalonamento e periodic thread" << endl;

    cout << "Criando threads" << endl;

    thread_a = new Periodic_Thread(RTConf(period_a * 1000, iterations), &func_a);
    thread_b = new Periodic_Thread(RTConf(period_b * 1000, iterations), &func_b);
    thread_c = new Periodic_Thread(RTConf(period_c * 1000, iterations), &func_c);

    exec('M');

    chrono.start();

    int status_a = thread_a->join();
    int status_b = thread_b->join();
    int status_c = thread_c->join();

    chrono.stop();

    exec('M');

    cout << "\n\nThread A encerrou com status \"" << char(status_a)
         << "\", thread B encerrou com status \"" << char(status_b)
         << "\" and thread C encerrou com status \"" << char(status_c) << "." << endl;

    cout << "\nTempo estimado para rodar o teste foi de "
         << max(period_a, period_b, period_c) * iterations
         << " ms. o tempo medido foi " << chrono.read() / 1000 <<" ms!" << endl;
}

int func_a()
{
    exec('A');

    do {
        exec('a', wcet_a);
    } while (Periodic_Thread::wait_next());

    exec('A');

    return 'A';
}

int func_b()
{
    exec('B');

    do {
        exec('b', wcet_b);
    } while (Periodic_Thread::wait_next());

    exec('B');

    return 'B';
}

int func_c()
{
    exec('C');

    do {
        exec('c', wcet_c);
    } while (Periodic_Thread::wait_next());

    exec('C');

    return 'C';
}