#include "scheduler.h"
#include "../process/process.h"
#include <stdio.h>

void schedule_fcfs() {
  printf("FCFS Scheduling:\n");
  for (int i = 0; i < process_count; i++) {
    printf("Process %d running (CPU burst %d)\n", process_table[i].pid,
           process_table[i].cpu_burst);
    process_table[i].cpu_burst = 0;
  }
}

void schedule_rr(int quantum) {
  printf("Round Robin Scheduling (Quantum=%d):\n", quantum);
  int done = 0;
  while (!done) {
    done = 1;
    for (int i = 0; i < process_count; i++) {
      if (process_table[i].cpu_burst > 0) {
        done = 0;
        int run = (process_table[i].cpu_burst < quantum)
                      ? process_table[i].cpu_burst
                      : quantum;
        process_table[i].cpu_burst -= run;
        printf("Process %d ran for %d units, remaining %d\n",
               process_table[i].pid, run, process_table[i].cpu_burst);
      }
    }
  }
}