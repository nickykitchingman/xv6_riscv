#include "kernel/types.h"
#include "kernel/spinlock.h"
#include "thread_switch.S"

struct thread_context
{
    uint64 ra;
    uint64 sp;
    uint64 fp; // s0
    uint64 s1;
    uint64 s2;
    uint64 s3;
    uint64 s4;
    uint64 s5;
    uint64 s6;
    uint64 s7;
    uint64 s8;
    uint64 s9;
    uint64 s10;
    uint64 s11;
};

enum threadstate
{
    TUNUSED,
    TUSED,
    TSLEEPING,
    TRUNNABLE,
    TRUNNING
};

struct thread
{
    struct spinlock lock;

    enum threadstate state;
    struct thread_context *context;
    void *chan;
};

struct thread *thread_create();
void thread_switch(struct thread_context *, struct thread_context *);
void thread_join(struct thread *);

#define NTHREAD 64