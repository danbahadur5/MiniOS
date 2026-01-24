#include "../process/process.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
static time_t start_time;
void init_system_monitor() { start_time = time(NULL); }
void show_cpu_usage() { printf("CPU Usage: %d%%\n", rand() % 100); }
void show_process_stats() {
  printf("PID\tSTATE\n");
  for (int i = 0; i < process_count; i++)
    printf("%d\t%s\n", process_table[i].pid, process_table[i].state);
}
void show_uptime() {
  time_t now = time(NULL);
  printf("System Uptime: %d seconds\n", (int)difftime(now, start_time));
}