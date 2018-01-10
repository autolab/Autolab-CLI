#include <stdio.h>
#include <stdlib.h>

void err_assert(bool value) {
  if (!value) {
    perror("Fatal Error");
    exit(0);
  }
}

void err_assert(bool value, const char *msg) {
  if (!value) {
    printf("Fatal Error: %s\n", msg);
    exit(0);
  }
}