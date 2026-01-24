#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Process process_table[MAX_PROCESS];
int process_count = 0;

void create_process() {
  if (process_count >= MAX_PROCESS) {
    printf("Cannot create more processes.\n");
    return;
  }
  Process p;
  p.pid = process_count + 1;
  strcpy(p.state, "READY");
  p.priority = 1;
  p.cpu_burst = rand() % 10 + 1;
  process_table[process_count++] = p;
  printf("Process %d created with CPU burst %d.\n", p.pid, p.cpu_burst);
}

void terminate_process(int pid) {
  int found = 0;
  for (int i = 0; i < process_count; i++) {
    if (process_table[i].pid == pid) {
      found = 1;
      for (int j = i; j < process_count - 1; j++)
        process_table[j] = process_table[j + 1];
      process_count--;
      printf("Process %d terminated.\n", pid);
      break;
    }
  }
  if (!found)
    printf("Process %d not found.\n", pid);
}

void suspend_process(int pid) {
  for (int i = 0; i < process_count; i++) {
    if (process_table[i].pid == pid) {
      strcpy(process_table[i].state, "BLOCKED");
      printf("Process %d suspended (BLOCKED).\n", pid);
      return;
    }
  }
  printf("Process %d not found.\n", pid);
}

void resume_process(int pid) {
  for (int i = 0; i < process_count; i++) {
    if (process_table[i].pid == pid) {
      strcpy(process_table[i].state, "READY");
      printf("Process %d resumed (READY).\n", pid);
      return;
    }
  }
  printf("Process %d not found.\n", pid);
}

void list_processes() {
  printf("----------------------------------------------------------\n");
  printf(" %-5s | %-10s | %-8s | %-10s \n", "PID", "STATE", "PRIORITY",
         "CPU BURST");
  printf("----------------------------------------------------------\n");
  for (int i = 0; i < process_count; i++) {
    printf(" %-5d | %-10s | %-8d | %-10d \n", process_table[i].pid,
           process_table[i].state, process_table[i].priority,
           process_table[i].cpu_burst);
  }
  printf("----------------------------------------------------------\n");
}