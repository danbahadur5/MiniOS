#include "memory.h"
#include <stdio.h>
#define MEMORY_SIZE 1024
int memory[MEMORY_SIZE];

void allocate_memory(int size) {
  int count = 0;
  for (int i = 0; i < MEMORY_SIZE; i++) {
    if (memory[i] == 0)
      count++;
    else
      count = 0;
    if (count == size) {
      for (int j = i - size + 1; j <= i; j++)
        memory[j] = 1;
      printf("Allocated %d KB memory.\n", size);
      return;
    }
  }
  printf("Not enough memory.\n");
}

void free_memory(int size) {
  int freed = 0;
  for (int i = 0; i < MEMORY_SIZE && freed < size; i++) {
    if (memory[i] == 1) {
      memory[i] = 0;
      freed++;
    }
  }
  printf("Freed %d KB memory.\n", freed);
}

void show_memory_usage() {
  int used = 0;
  for (int i = 0; i < MEMORY_SIZE; i++)
    if (memory[i] == 1)
      used++;
  printf("Memory Used: %d KB, Free: %d KB\n", used, MEMORY_SIZE - used);
}