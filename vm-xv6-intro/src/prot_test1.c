#include "types.h"
#include "user.h"

int main(int argc, char *argv[]) {
  char *addr = malloc(4096);
  if (addr == 0) {
    printf(1, "Error: malloc\n");
    exit();
  }
  printf(1, "allocated page at address %p\n", addr);
  if (mprotect(addr, 1) < 0) {
    printf(1, "Error: mprotect failed\n");
    exit();
  }
  printf(1, "if a trap occurred, mprotect test passed\n");
  addr[0] = 'A'; /* a page fault */
  if (munprotect(addr, 1)<0) {
    printf(1, "munprotect failed\n");
  }
  addr[0]='A';
  printf(1, "munprotect succeeded\n");
  exit();
}
