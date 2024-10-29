#include "types.h"
#include "stat.h"
#include "user.h"

int main(void) {
	int x = 9;
    int *ptr = 0x0; // Null pointer
    int value;

    // Attempt to dereference the null pointer
    printf(1, "Attempting to access null pointer...\n");

    // This will likely cause a crash
    value = *ptr;

    // If somehow no crash, print the value (highly unlikely)
    printf(1, "Null pointer value: %d\n", value);

    exit();
}

