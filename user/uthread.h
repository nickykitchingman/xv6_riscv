#include "kernel/types.h"
#include "thread_switch.S"

struct thread
{
};

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

struct thread *thread_create();
void thread_switch(struct thread_context *, struct thread_context *);
void thread_join(struct thread *);