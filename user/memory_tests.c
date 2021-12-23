#include "kernel/types.h"
#include "user/user.h"
#include "memory_management.h"

int test(int (*testFunc)()) {
    if (fork() == 0) {
        int xstatus = testFunc();
        exit(xstatus);
    } else {
        int xstatus;
        wait(&xstatus);
        if(xstatus == -1){
            // probably page fault, so might be lazy lab,
            // so OK.
            printf("FAIL (page fault)\n");
        } else if (xstatus == 0)
            printf("OK\n");
        else
            printf("FAIL\n");
        return xstatus;
    }

}

int mallocFreeTest() {
    printf("Malloc Free test\n");
    void * start = sbrk(0);

    void *m1, *m2;
    printf("malloc test\n");
    m1 = _malloc(10001);
    m2 = _malloc(10001);
    printf("malloc success\n");

    printf("free test\n");
    _free(m1);
    _free(m2);

    void *end = sbrk(0);

    if (end == start) {
        printf("free success\n");
        return 0;;
    } else {
        printf("free fail sbrk(0) = %d\n", end);
        return 1;
    }
    return 0;
}

// int freeMergeTest() {
//     printf("Free Merge Test\n");

//     void* m1 = _malloc(10001);
//     void* m2 = _malloc(10002);
//     void* m3 = _malloc(10003);

//     _free(m2);
//     _free(m1);

//     if (base->s.nextHeader != m3-sizeof(header)) {
//         _free(m3);
//         return 1;
//     }
//     _free(m3);
//     return 0;
// }

int insertBlockTest() {
    printf("Insert Block Test\n");
    
    void* m1 = _malloc(5000);
    void* m2 = _malloc(2000);
    void* m3 = _malloc(3000);
    void* m4 = _malloc(5000);

    long topBeforeInsertion = (long)sbrk(0);

    _free(m1);
    _free(m3);
    void* m5 = _malloc(2000);

    long topAfterInsertion = (long)sbrk(0);

    int status = 0;
    if (topBeforeInsertion != topAfterInsertion)
        status = 1;
    
    _free(m2);
    _free(m4);
    _free(m5);

    return status;
}

int allocateLargeTest() {
    printf("Allocate Large Test\n");

    int status = 0;
    void *start = sbrk(0);

    void* m1 = _malloc(1000000);
    void* m2 = _malloc(1);

    if (m2 != m1 + 1000000 + sizeof(header))
        status = 1;

    _free(m2);
    _free(m1);

    void *end = sbrk(0);

    if (start != end)
        status = 1;
    return status;
}

int memTest() {
    // Allocate all mem, free it, and allocate again

    void *m1, *m2;

    m1 = 0;
    while((m2 = _malloc(10001)) != 0){
        *(char**)m2 = m1;
        m1 = m2;
    }
    while(m1){
        m2 = *(char**)m1;
        _free(m1);
        m1 = m2;
    }
    m1 = _malloc(1024*20);
    if(m1 == 0){
        printf("couldn't allocate mem?!!\n");
        return 1;
    }
    _free(m1);
    return 0;
}

int main(int argc, char**argv) {
    int status = 0;
    status |= test(mallocFreeTest);
    //status |= test(freeMergeTest);
    status |= test(insertBlockTest);
    status |= test(allocateLargeTest);
    //status |= test(memTest);

    if (status == 0)
        printf("ALL TESTS PASSED\n");
    else 
        printf("TESTS FAILED\n");

    exit(0);
}