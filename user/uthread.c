#include "uthread.h"

struct thread threads[NTHREAD];

struct thread *tcurrent;

// initialize the proc table at boot time.
void threadinit(void)
{
    struct thread *t;

    for (t = threads; t < &threads[NTHREAD]; t++)
    {
        initlock(&t->lock, "thread");
    }
}

struct thread *
thread_create()
{
    struct thread *t;

    for (t = threads; t < &threads[NTHREAD]; t++)
    {
        acquire(&t->lock);
        if (t->state == TUNUSED)
        {
            release(&t->lock);
            goto found;
        }
    }
    return 0;
found:

    t->state = TUSED;

    memset(t->context, 0, sizeof(t->context));
    // return address
    // stack pointer
}

void thread_scheduler()
{
    struct thread *t;

    for (;;)
    {
        for (t = threads; t < &threads[NTHREAD]; t++)
        {
            acquire(&t->lock);
            thread_switch(tcurrent->context, &t->context);
            release(&t->lock);
        }
    }
}

void thread_sched()
{
}

void thread_join(struct thread *t)
{
    if (t == tcurrent)
        panic("thread join with current thread");

    acquire(tcurrent->lock);
    tcurrent->state = TSLEEPING;
    release(tcurrent->lock);

    acquire(t->lock);
    while (t->state == TRUNNING)
    {
        release(t->lock);
        thread_sched();
        acquire(t->lock);
    }
    release(t->lock);

    acquire(tcurrent->lock);
    tcurrent->state = TRUNNING;
    release(tcurrent->lock);
}