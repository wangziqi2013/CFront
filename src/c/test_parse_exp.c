
#include <stdio.h>

int main() {
  printf("Hello World!\n");
  perror(__func__);
  return 0;
}