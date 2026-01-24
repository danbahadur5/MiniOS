#include "../filesystem/filesystem.h"
#include "../memory/memory.h"
#include "../monitor/monitor.h"
#include "../process/process.h"
#include "../scheduler/scheduler.h"
#include "../security/auth.h"
#include "../sync/sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ANSI Color Codes
#define COLOR_RESET "\033[0m"
#define COLOR_RED "\033[31m"
#define COLOR_GREEN "\033[32m"
#define COLOR_YELLOW "\033[33m"
#define COLOR_BLUE "\033[34m"
#define COLOR_CYAN "\033[36m"
#define COLOR_BOLD "\033[1m"
#define COLOR_MAGENTA "\033[35m"

#define MAX_HISTORY 10
char history[MAX_HISTORY][100];
int history_count = 0;

void add_to_history(const char *cmd) {
  if (history_count < MAX_HISTORY) {
    strcpy(history[history_count++], cmd);
  } else {
    for (int i = 0; i < MAX_HISTORY - 1; i++) {
      strcpy(history[i], history[i + 1]);
    }
    strcpy(history[MAX_HISTORY - 1], cmd);
  }
}

void print_history() {
  printf(COLOR_MAGENTA "Command History:\n" COLOR_RESET);
  for (int i = 0; i < history_count; i++) {
    printf("  %d: %s\n", i + 1, history[i]);
  }
}

void cmd_date() {
  time_t t = time(NULL);
  struct tm tm = *localtime(&t);
  printf("Current System Time: %04d-%02d-%02d %02d:%02d:%02d\n",
         tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min,
         tm.tm_sec);
}

void print_banner() {
  printf(COLOR_CYAN COLOR_BOLD);
  printf("  __  __ _       _  ____  _____\n");
  printf(" |  \\/  (_)     (_)/ __ \\/ ____|\n");
  printf(" | \\  / |_ _ __  _| |  | | (___\n");
  printf(" | |\\/| | | '_ \\| | |  | |\\___ \\\n");
  printf(" | |  | | | | | | | |__| |____) |\n");
  printf(" |_|  |_|_|_| |_|_|\\____/|_____/\n");
  printf(COLOR_RESET);
  printf("      v1.0 - Ultimate System\n\n");
}

void help_command() {
  printf(COLOR_YELLOW "Available Commands:\n" COLOR_RESET);
  printf("  " COLOR_BOLD "System:" COLOR_RESET "\n");
  printf("    help                - Show this help message\n");
  printf("    clear               - Clear the screen\n");
  printf("    shutdown            - Shutdown the OS (Admin only)\n");
  printf("    login               - Login to the system\n");
  printf("    logout              - Logout current user\n");
  printf("    whoami              - Show current user\n");
  printf("    history             - Show command history\n");
  printf("    date                - Show current date/time\n");
  printf("    echo <text>         - Print text to screen\n");

  printf("  " COLOR_BOLD "Process Management:" COLOR_RESET "\n");
  printf("    ps                  - List all processes\n");
  printf("    create_process      - Create a new process\n");
  printf("    kill <pid>          - Terminate a process\n");
  printf("    suspend <pid>       - Suspend (block) a process\n");
  printf("    resume <pid>        - Resume (unblock) a process\n");
  printf("    schedule            - Run FCFS scheduler\n");
  printf("    top                 - Show CPU and process stats\n");

  printf("  " COLOR_BOLD "Memory Management:" COLOR_RESET "\n");
  printf("    mem                 - Show memory usage\n");
  printf("    malloc <size>       - Allocate memory\n");
  printf("    free <size>         - Free memory\n");

  printf("  " COLOR_BOLD "Filesystem:" COLOR_RESET "\n");
  printf("    ls                  - List files\n");
  printf("    touch <name>        - Create a file\n");
    printf("    rm <name>           - Delete a file\n");
    printf("    mv <old> <new>      - Rename a file\n");
    printf("    cp <src> <dest>     - Copy a file\n");
    printf("    write <name> <txt>  - Write text to a file\n");
  printf("    cat <name>          - Read file content\n");

  printf("  " COLOR_BOLD "Concurrency:" COLOR_RESET "\n");
  printf("    producer            - Run Producer-Consumer demo\n");
  printf("    consumer            - Run Consumer demo\n");
  printf("    reader              - Run Reader demo\n");
  printf("    writer              - Run Writer demo\n");
  printf("    banker              - Run Banker's Algorithm demo\n");
}

void clear_screen() {
#ifdef _WIN32
  system("cls");
#else
  system("clear");
#endif
}

void start_cli() {
  char input[100];
  char raw_input[100]; // Keep raw input for history
  char *command;
  char *arg1;
  char *arg2;

  clear_screen();
  print_banner();

  while (1) {
    // Prompt
    printf(COLOR_GREEN "[%s@MiniOS]" COLOR_RESET "$ ", get_current_user());

    if (fgets(input, sizeof(input), stdin) == NULL) {
      break; // EOF
    }

    // Remove newline
    input[strcspn(input, "\n")] = 0;

    // Skip empty lines
    if (strlen(input) == 0)
      continue;

    // Save to history (copy before strtok)
    strcpy(raw_input, input);
    add_to_history(raw_input);

    // Tokenize
    command = strtok(input, " ");
    arg1 = strtok(NULL, " ");
    arg2 = strtok(NULL, ""); // Get the rest of the string for arg2 (useful for write content)

    if (command == NULL)
      continue;

    // --- System Commands ---
    if (strcmp(command, "help") == 0) {
      help_command();
    } else if (strcmp(command, "clear") == 0) {
      clear_screen();
      print_banner();
    } else if (strcmp(command, "history") == 0) {
      print_history();
    } else if (strcmp(command, "date") == 0) {
      cmd_date();
    } else if (strcmp(command, "echo") == 0) {
      if (arg1) {
        printf("%s", arg1);
        if (arg2)
          printf(" %s", arg2);
        printf("\n");
      } else {
        printf("\n");
      }
    } else if (strcmp(command, "login") == 0) {
      login();
    } else if (strcmp(command, "logout") == 0) {
      logout();
    } else if (strcmp(command, "whoami") == 0) {
      printf("Current user: " COLOR_BOLD "%s" COLOR_RESET "\n",
             get_current_user());
    } else if (strcmp(command, "shutdown") == 0) {
      if (!is_admin()) {
        printf(COLOR_RED "Error: Admin privileges required!\n" COLOR_RESET);
      } else {
        printf("Shutting down...\n");
        break;
      }
    }
    // --- Process Commands ---
    else if (strcmp(command, "ps") == 0) {
      list_processes();
    } else if (strcmp(command, "create_process") == 0) {
      create_process();
    } else if (strcmp(command, "kill") == 0) {
      if (arg1) {
        int pid = atoi(arg1);
        terminate_process(pid);
      } else {
        printf(COLOR_RED "Usage: kill <pid>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "suspend") == 0) {
      if (arg1) {
        int pid = atoi(arg1);
        suspend_process(pid);
      } else {
        printf(COLOR_RED "Usage: suspend <pid>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "resume") == 0) {
      if (arg1) {
        int pid = atoi(arg1);
        resume_process(pid);
      } else {
        printf(COLOR_RED "Usage: resume <pid>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "schedule") == 0) {
      schedule_fcfs();
    } else if (strcmp(command, "top") == 0) {
      show_cpu_usage();
      show_process_stats();
    } else if (strcmp(command, "uptime") == 0) {
      show_uptime();
    }
    // --- Memory Commands ---
    else if (strcmp(command, "mem") == 0) {
      show_memory_usage();
    } else if (strcmp(command, "malloc") == 0) {
      if (arg1) {
        allocate_memory(atoi(arg1));
      } else {
        printf(COLOR_RED "Usage: malloc <size>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "free") == 0) {
      if (arg1) {
        free_memory(atoi(arg1));
      } else {
        printf(COLOR_RED "Usage: free <size>\n" COLOR_RESET);
      }
    }
    // --- Filesystem Commands ---
    else if (strcmp(command, "ls") == 0) {
      list_files();
    } else if (strcmp(command, "touch") == 0) {
      if (arg1) {
        touch_file(arg1);
      } else {
        printf(COLOR_RED "Usage: touch <filename>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "rm") == 0) {
      if (arg1) {
        delete_file(arg1);
      } else {
        printf(COLOR_RED "Usage: rm <filename>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "mv") == 0) {
      if (arg1 && arg2) {
        rename_file(arg1, arg2);
      } else {
        printf(COLOR_RED "Usage: mv <old_name> <new_name>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "write") == 0) {
      if (arg1 && arg2) {
        write_file(arg1, arg2);
      } else {
        printf(COLOR_RED "Usage: write <filename> <content>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "cat") == 0) {
      if (arg1) {
        read_file(arg1);
      } else {
        printf(COLOR_RED "Usage: cat <filename>\n" COLOR_RESET);
      }
    }
    // --- Concurrency ---
    else if (strcmp(command, "producer") == 0)
      producer();
    else if (strcmp(command, "consumer") == 0)
      consumer();
    else if (strcmp(command, "reader") == 0)
      reader(1);
    else if (strcmp(command, "writer") == 0)
      writer(1);
    else if (strcmp(command, "banker") == 0)
      bankers_algorithm();
    else {
      printf(COLOR_RED "Unknown command: %s\n" COLOR_RESET, command);
    }
  }
}
