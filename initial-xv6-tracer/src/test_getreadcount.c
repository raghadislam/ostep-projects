#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
    int count = getreadcount();  // Call your new system call
    printf(1, "The read system call has been called %d times.\n", count);
    exit();
}

