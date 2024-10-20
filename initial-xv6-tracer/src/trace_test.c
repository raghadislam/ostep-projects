#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
    trace("testfile");  // Start tracing "testfile"
    
    // Open the file multiple times
    for(int i = 0; i < 3; i++) {
        open("testfile", 0);  // Open traced file
    }

    // Open a file that is not traced
    open("otherfile", 0);

    printf(1, "Trace count for 'testfile': %d\n", getcount());  // Should be 3
    exit();
}

