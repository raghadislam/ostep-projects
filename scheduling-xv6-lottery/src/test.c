#include "types.h"
#include "user.h"
#include "pstat.h"

void process(void) {
  while (1) {}
  exit();
}

struct pstat p;

int main(void) {
  int i;

  // Create three child processes with different numbers of tickets
  int pid_A = fork();
  if (pid_A == 0) {
    settickets(30);
    process();
  }

  int pid_B = fork();
  if (pid_B == 0) {
    settickets(20);
    process();
  }

  int pid_C = fork();
  if (pid_C == 0) {
    settickets(10);
    process();
  }

  // Collect data every few seconds
  for (i = 0; i < 100; i++) {
    sleep(100); // Adjust this value for frequency of data collection
    if (getpinfo(&p) < 0) {
      printf(1, "Error: getpinfo failed\n");
      exit();
    }
    printf(1, "Tick count at iteration %d:\n", i);
    for (int j = 0; j < NPROC; j++) {
      if (p.inuse[j]) {
        printf(1, "PID %d: %d ticks\n", p.pid[j], p.ticks[j]);
      }
    }
  }

  // Wait for the child processes to exit
  for (i = 0; i < 3; i++) {
    wait();
  }

  exit();
}

