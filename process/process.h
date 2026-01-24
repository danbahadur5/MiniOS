#ifndef PROCESS_H
#define PROCESS_H
#define MAX_PROCESS 100
typedef struct {
  int pid;
  char state[10];
  int priority;
  int cpu_burst;
} Process;
extern Process process_table[MAX_PROCESS];
extern int process_count;
void create_process();
void terminate_process(int pid);
void suspend_process(int pid);
void resume_process(int pid);
void list_processes();
#endif