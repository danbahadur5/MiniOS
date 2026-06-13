#include "process.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <shellapi.h>

Process process_table[MAX_PROCESS];
int process_count = 0;

typedef struct
{
  const char *name;
  const char *description;
  int cpu_burst;
} SoftwareEntry;

static SoftwareEntry software_catalog[] = {
    {"notepad", "Windows Notepad", 4},
    {"wordpad", "Windows WordPad", 5},
    {"mspaint", "Paint", 5},
    {"calculator", "Windows Calculator", 4},
    {"taskmgr", "Task Manager", 5},
    {"powershell", "Windows PowerShell", 6},
    {"cmd", "Command Prompt", 5},
    {"control", "Control Panel", 5},
    {"chrome", "Google Chrome browser", 8},
    {"edge", "Microsoft Edge browser", 8},
    {"explorer", "File Explorer", 5},
    {"terminal", "Terminal app", 5},
};

static const SoftwareEntry *find_software(const char *name)
{
  for (int i = 0; i < (int)(sizeof(software_catalog) / sizeof(software_catalog[0])); i++)
  {
    if (strcmp(software_catalog[i].name, name) == 0)
    {
      return &software_catalog[i];
    }
  }
  return NULL;
}

void create_process()
{
  if (process_count >= MAX_PROCESS)
  {
    printf("Cannot create more processes.\n");
    return;
  }
  Process p;
  p.pid = process_count + 1;
  strcpy(p.name, "process");
  strcpy(p.state, "READY");
  p.priority = 1;
  p.cpu_burst = rand() % 10 + 1;
  process_table[process_count++] = p;
  printf("Process %d created with CPU burst %d.\n", p.pid, p.cpu_burst);
}

void open_software(const char *name)
{
  if (process_count >= MAX_PROCESS)
  {
    printf("Cannot open more software.\n");
    return;
  }

  const SoftwareEntry *entry = find_software(name);
  if (!entry)
  {
    printf("Unknown software: %s\n", name);
    list_available_software();
    return;
  }

  HINSTANCE result = ShellExecuteA(NULL, "open", entry->name, NULL, NULL, SW_SHOWNORMAL);
  LONG_PTR code = (LONG_PTR)result;
  if (code <= 32)
  {
    printf("Failed to launch %s (ShellExecute error %ld).\n", entry->name, (long)code);
    return;
  }

  Process p;
  p.pid = process_count + 1;
  strncpy(p.name, entry->name, sizeof(p.name) - 1);
  p.name[sizeof(p.name) - 1] = '\0';
  strcpy(p.state, "READY");
  p.priority = 1;
  p.cpu_burst = entry->cpu_burst;
  process_table[process_count++] = p;
  printf("Opened software '%s' (%s) as process %d.\n",
         entry->name, entry->description, p.pid);
}

void list_available_software()
{
  printf("Available software:\n");
  for (int i = 0; i < (int)(sizeof(software_catalog) / sizeof(software_catalog[0])); i++)
  {
    printf("  - %s: %s\n", software_catalog[i].name, software_catalog[i].description);
  }
}

void terminate_process(int pid)
{
  int found = 0;
  for (int i = 0; i < process_count; i++)
  {
    if (process_table[i].pid == pid)
    {
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

void list_processes()
{
  printf("----------------------------------------------------------\n");
  printf(" %-5s | %-12s | %-10s | %-8s | %-10s \n", "PID", "NAME", "STATE", "PRIORITY",
         "CPU BURST");
  printf("----------------------------------------------------------\n");
  for (int i = 0; i < process_count; i++)
  {
    printf(" %-5d | %-12s | %-10s | %-8d | %-10d \n", process_table[i].pid,
           process_table[i].name, process_table[i].state, process_table[i].priority,
           process_table[i].cpu_burst);
  }
  printf("----------------------------------------------------------\n");
}