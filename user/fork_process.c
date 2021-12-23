#include "kernel/types.h"
#include "user/user.h"

int main (int argc, char *argv[]) 
{
    int num = atoi(argv[1]);

    int p[2];
    int procNum;
    pipe(p);

    for (int i = 1; i <= num; i++) {
        if (fork() == 0) {
            int n = read(p[0], &procNum, sizeof(int));
            if (n < 0) {
                // close(1);
                // dup(p[1]);
                printf("Read error\n");
            }
            else {
                // close(1);
                // dup(p[1]);
                printf("Hello from process %d\n", procNum);
            }
            exit(0);
        }
        else {
            write(p[1], &i, sizeof(int));
            wait((int *) 0);
        }
    }

    // close(p[1]);
    // char str[256];
    // int n;
    // do {
    //     n = read(p[0], str, sizeof str);
    //     printf(str);
    // } while (n > 0);

    exit(0);
}