#include "../ai/ai.h"
#include "../filesystem/filesystem.h"
#include "../monitor/monitor.h"
#include "../process/process.h"
#include "../security/auth.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

#define MAX_ALIASES 50
typedef struct {
   char name[32]; 
   char cmd[200]; 
   char owner[20]; 
   int system; 
  } Alias;
static Alias aliases[MAX_ALIASES];
static int alias_count = 0;

static Alias *find_alias(const char *name) {
  if (!name) return NULL;
  for (int i = 0; i < alias_count; i++) {
    if (strcmp(aliases[i].name, name) == 0) return &aliases[i];
  }
  return NULL;
}

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
  printf("    h & help            - Show this help message\n");
  printf("    clear & cls         - Clear the screen\n");
  printf("    shutdown            - Shutdown the OS\n");
  printf("    login               - Login to the system\n");
  printf("    logout              - Logout current user\n");
  printf("    whoami              - Show current user\n");
  printf("    history             - Show command history\n");
  printf("    date                - Show current date/time\n");
  printf("    echo <text>         - Print text to screen\n");

  printf("  " COLOR_BOLD "Process Management:" COLOR_RESET "\n");
  printf("    ps                  - List all processes\n");
  printf("    open <software>     - Open a software app\n");
  printf("    software            - List available software apps\n");
  printf("    kill <pid>          - Terminate a process\n");
  printf("    top                 - Show CPU and process stats\n");

  printf("  " COLOR_BOLD "Filesystem:" COLOR_RESET "\n");
  printf("    ls                  - List files\n");
  printf("    touch <name>        - Create a file\n");
  printf("    rm <name>           - Delete a file\n");
  printf("    mv <old> <new>      - Rename a file\n");
  printf("    write <name> <txt>  - Write text to a file\n");
  printf("    cat <name>          - Read file content\n");
  printf("    show <name>         - Show file content\n");
  printf("    share <name> <enable|disable> - Set file share (read-only for others)\n");
  printf("    mkdir <name>        - Create directory\n");
  printf("    rmdir <name>        - Remove directory (must be empty)\n");

  printf("  " COLOR_BOLD "AI & Automation:" COLOR_RESET "\n");
  printf("    ai ask <question>        - Ask Ollama AI a question\n");
  printf("    ai summarize <text>      - Summarize text using AI\n");
  printf("    ai automate <task>       - Suggest MiniOS commands for a task\n");
  printf("    ai translate <text> to <lang> - Translate text to a language\n");
  printf("    ai explain <topic>       - Explain a concept or command\n");
  printf("    ai code <description>    - Generate C code\n");
  printf("    ai fix <error>           - Diagnose and fix an error\n");
  printf("    ai chat <message>        - Multi-turn conversation with AI\n");
  printf("    ai chat reset            - Clear chat history\n");
  printf("    ai models                - List available AI models\n");
  printf("    ai model <name>          - Set active AI model\n");
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
  char raw_input[100];
  char *command;
  char *arg1;
  char *arg2;

  clear_screen();
  print_banner();

  while (1) {
    printf(COLOR_GREEN "[%s@MiniOS]" COLOR_RESET "$ ", get_current_user());

    if (fgets(input, sizeof(input), stdin) == NULL) {
      break;
    }

    input[strcspn(input, "\r")] = 0;
    input[strcspn(input, "\n")] = 0;

    if (strlen(input) == 0)
      continue;

    strcpy(raw_input, input);
    add_to_history(raw_input);

    char peek[256];
    strcpy(peek, raw_input);
    char *peek_cmd = strtok(peek, " ");
    char *peek_args = strtok(NULL, "");
    Alias *peek_alias = find_alias(peek_cmd);
    if (peek_alias) {
      char expanded[512];
      expanded[0] = '\0';
      strncat(expanded, peek_alias->cmd, sizeof(expanded)-1);
      if (peek_args) {
        strncat(expanded, " ", sizeof(expanded)-strlen(expanded)-1);
        strncat(expanded, peek_args, sizeof(expanded)-strlen(expanded)-1);
      }
      strcpy(input, expanded);
    }

    command = strtok(input, " ");
    arg1 = strtok(NULL, " ");
    arg2 = strtok(NULL, "");

    if (command == NULL)
      continue;
      
    if (strcmp(command, "h") == 0 || strcmp(command, "help") == 0) {
      help_command();
    } else if (strcmp(command, "clear") == 0 || strcmp(command, "cls") == 0) {
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
      printf("Shutting down...\n");
      break;
    } else if (strcmp(command, "ps") == 0) {
      list_processes();
    } else if (strcmp(command, "kill") == 0) {
      if (arg1) {
        int pid = atoi(arg1);
        terminate_process(pid);
      } else {
        printf(COLOR_RED "Usage: kill <pid>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "top") == 0) {
      show_cpu_usage();
      show_process_stats();
    } else if (strcmp(command, "open") == 0) {
      if (arg1) {
        open_software(arg1);
      } else {
        printf(COLOR_RED "Usage: open <software>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "software") == 0) {
      list_available_software();
    } else if (strcmp(command, "uptime") == 0) {
      show_uptime();
    } else if (strcmp(command, "ls") == 0) {
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
    } else if (strcmp(command, "show") == 0) {
      if (arg1) {
        read_file(arg1);
      } else {
        printf(COLOR_RED "Usage: show <filename>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "cat") == 0) {
      if (arg1) {
        read_file(arg1);
      } else {
        printf(COLOR_RED "Usage: cat <filename>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "mkdir") == 0) {
      if (arg1) {
        make_dir(arg1);
      } else {
        printf(COLOR_RED "Usage: mkdir <name>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "rmdir") == 0) {
      if (arg1) {
        remove_dir(arg1);
      } else {
        printf(COLOR_RED "Usage: rmdir <name>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "share") == 0) {
      if (arg1 && arg2) {
        char *mode = arg2;
        while (*mode == ' ') mode++;
        if (strcmp(mode, "enable") == 0) {
          set_file_share(arg1, 1);
        } else if (strcmp(mode, "disable") == 0) {
          set_file_share(arg1, 0);
        } else {
          printf(COLOR_RED "Usage: share <filename> <enable|disable>\n" COLOR_RESET);
        }
      } else {
        printf(COLOR_RED "Usage: share <filename> <enable|disable>\n" COLOR_RESET);
      }
    } else if (strcmp(command, "ai") == 0) {
      if (!arg1) {
        printf(COLOR_RED "Usage: ai <ask|summarize|automate|translate|explain|code|fix|chat|models|model>\n" COLOR_RESET);
      } else if (strcmp(arg1, "models") == 0) {
        const char *avail = ai_get_available_models();
        const char *cur = ai_get_model();
        if (strlen(avail) > 0) {
          printf("Available models: %s\n", avail);
        } else {
          printf("No models found. Pull one with: ollama pull <model>\n");
        }
        if (strlen(cur) > 0) {
          printf("Active model: %s\n", cur);
        }
      } else if (strcmp(arg1, "model") == 0) {
        if (arg2) {
          if (ai_set_model(arg2)) {
            printf("Active model set to: %s\n", arg2);
          } else {
            printf(COLOR_RED "Failed to set model.\n" COLOR_RESET);
          }
        } else {
          printf("Current model: %s\n", ai_get_model());
        }
      } else if (strcmp(arg1, "ask") == 0) {
        if (arg2) {
          char response[8192];
          fflush(stdout);
          if (ai_ask(arg2, response, sizeof(response))) {
            printf("\n" COLOR_CYAN "AI Response:" COLOR_RESET " %s\n", response);
          } else {
            printf(COLOR_RED "AI request failed.\n" COLOR_RESET);
          }
        } else {
          printf(COLOR_RED "Usage: ai ask <question>\n" COLOR_RESET);
        }
      } else if (strcmp(arg1, "summarize") == 0) {
        if (arg2) {
          char summary[8192];
          if (ai_summarize(arg2, summary, sizeof(summary))) {
            printf("\n" COLOR_CYAN "Summary:" COLOR_RESET " %s\n", summary);
          } else {
            printf(COLOR_RED "Summarization failed.\n" COLOR_RESET);
          }
        } else {
          printf(COLOR_RED "Usage: ai summarize <text>\n" COLOR_RESET);
        }
      } else if (strcmp(arg1, "automate") == 0) {
        if (arg2) {
          char suggestion[8192];
          if (ai_automate(arg2, suggestion, sizeof(suggestion))) {
            printf("\n" COLOR_CYAN "Automation:" COLOR_RESET " %s\n", suggestion);
          } else {
            printf(COLOR_RED "Automation request failed.\n" COLOR_RESET);
          }
        } else {
          printf(COLOR_RED "Usage: ai automate <task description>\n" COLOR_RESET);
        }
      } else if (strcmp(arg1, "translate") == 0) {
        if (arg2) {
          const char *to_ptr = NULL, *p = arg2;
          while ((p = strstr(p, " to ")) != NULL) { to_ptr = p; p += 4; }
          if (to_ptr) {
            char text[2048], lang[256];
            size_t tlen = (size_t)(to_ptr - arg2);
            if (tlen >= sizeof(text)) tlen = sizeof(text) - 1;
            strncpy(text, arg2, tlen); text[tlen] = '\0';
            strncpy(lang, to_ptr + 4, sizeof(lang) - 1); lang[sizeof(lang) - 1] = '\0';
            char translation[8192];
            if (ai_translate(text, lang, translation, sizeof(translation))) {
              printf("\n" COLOR_CYAN "Translation:" COLOR_RESET " %s\n", translation);
            } else {
              printf(COLOR_RED "Translation failed.\n" COLOR_RESET);
            }
          } else {
            printf(COLOR_RED "Usage: ai translate <text> to <language>\n" COLOR_RESET);
          }
        } else {
          printf(COLOR_RED "Usage: ai translate <text> to <language>\n" COLOR_RESET);
        }
      } else if (strcmp(arg1, "explain") == 0) {
        if (arg2) {
          char explanation[8192];
          if (ai_explain(arg2, explanation, sizeof(explanation))) {
            printf("\n" COLOR_CYAN "Explanation:" COLOR_RESET " %s\n", explanation);
          } else {
            printf(COLOR_RED "Explanation request failed.\n" COLOR_RESET);
          }
        } else {
          printf(COLOR_RED "Usage: ai explain <topic>\n" COLOR_RESET);
        }
      } else if (strcmp(arg1, "code") == 0) {
        if (arg2) {
          char code[8192];
          if (ai_code(arg2, code, sizeof(code))) {
            printf("\n" COLOR_CYAN "Generated Code:" COLOR_RESET "\n%s\n", code);
          } else {
            printf(COLOR_RED "Code generation failed.\n" COLOR_RESET);
          }
        } else {
          printf(COLOR_RED "Usage: ai code <description>\n" COLOR_RESET);
        }
      } else if (strcmp(arg1, "fix") == 0) {
        if (arg2) {
          char fix[8192];
          if (ai_fix(arg2, fix, sizeof(fix))) {
            printf("\n" COLOR_CYAN "Fix Suggestion:" COLOR_RESET " %s\n", fix);
          } else {
            printf(COLOR_RED "Error diagnosis failed.\n" COLOR_RESET);
          }
        } else {
          printf(COLOR_RED "Usage: ai fix <error>\n" COLOR_RESET);
        }
      } else if (strcmp(arg1, "chat") == 0) {
        if (arg2 && strcmp(arg2, "reset") == 0) {
          ai_chat_reset();
        } else if (arg2) {
          char response[8192];
          fflush(stdout);
          if (ai_chat_send(arg2, response, sizeof(response))) {
            printf("\n" COLOR_CYAN "AI:" COLOR_RESET " %s\n", response);
          } else {
            printf(COLOR_RED "Chat request failed.\n" COLOR_RESET);
          }
        } else {
          printf("Chat session active. Type messages to continue.\n");
          printf("Use: ai chat reset to clear history.\n");
        }
      } else {
        printf(COLOR_RED "Unknown ai subcommand: %s\n" COLOR_RESET, arg1);
      }
    } else {
      printf(COLOR_RED "Unknown command: %s\n" COLOR_RESET, command);
    }
  }
}
