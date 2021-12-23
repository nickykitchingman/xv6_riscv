#include "kernel/types.h"
#include "user/user.h"

int main (int argc, char *argv[]) 
{
    char ball = 'o';

    int p[2];
    pipe(p);

    int pid = fork();
    //int id = 0;
    if (pid == 0) {
    //    id = 1;
        write(p[1], &ball, 1);
    }

    char data;
    while (1) {
        pid = getpid();
        read(p[0], &data, 1);
        printf("%d: %c\n", pid, data);
        sleep(10);
        write(p[1], &data, 1);
    }

    exit(0);
}