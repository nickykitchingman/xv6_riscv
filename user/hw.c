#include "kernel/types.h"
#include "user/user.h"

void greet() {
  printf("Hello world\n");
}

int main(int argc, char *argv[]) {
  greet();
  exit(0);
}
