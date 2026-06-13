#include "filesystem.h"
#include <stdio.h>
#include <string.h>
#include "../security/auth.h"

#define FS_FILENAME "minios_fs.dat"

typedef struct {
  char name[50];
  char content[200];
  char owner[20];
  int shared;
  int is_dir;
} File;
File files[50];
int file_count = 0;

void save_filesystem() {
    FILE *f = fopen(FS_FILENAME, "wb");
    if (f) {
        fwrite(&file_count, sizeof(int), 1, f);
        fwrite(files, sizeof(File), file_count, f);
        fclose(f);
    } else {
        printf("Error: Could not save filesystem.\n");
    }
}




void load_filesystem() {
    FILE *f = fopen(FS_FILENAME, "rb");
    if (f) {
        fread(&file_count, sizeof(int), 1, f);
        fread(files, sizeof(File), file_count, f);
        fclose(f);
        printf("Filesystem loaded (%d files).\n", file_count);
    } else {
        printf("No existing filesystem found. Initialized empty.\n");
    }
}

void touch_file(const char *name) {
  if (file_count >= 50) {
    printf("Filesystem full.\n");
    return;
  }
  strcpy(files[file_count].name, name);
  files[file_count].content[0] = '\0';
  strncpy(files[file_count].owner, get_current_user(), sizeof(files[file_count].owner)-1);
  files[file_count].owner[sizeof(files[file_count].owner)-1] = '\0';
  files[file_count].shared = 0;
  files[file_count].is_dir = 0;
  file_count++;
  printf("File '%s' created (owner: %s).\n", name, get_current_user());
  save_filesystem();
}

void write_file(const char *name, const char *content) {
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, name) == 0) {
      if (files[i].is_dir) {
        printf("Cannot write: '%s' is a directory.\n", name);
        return;
      }
      
      const char *cur = get_current_user();
      if (strcmp(files[i].owner, cur) != 0 && !is_admin()) {
        printf("Permission denied: only owner (%s) or admin can modify '%s'.\n", files[i].owner, name);
        return;
      }
      strncpy(files[i].content, content, sizeof(files[i].content)-1);
      files[i].content[sizeof(files[i].content)-1] = '\0';
      printf("Written to '%s'.\n", name);
      save_filesystem();
      return;
    }
  }
  if (file_count >= 50) {
    printf("Filesystem full.\n");
    return;
  }
  strcpy(files[file_count].name, name);
  strncpy(files[file_count].content, content, sizeof(files[file_count].content)-1);
  files[file_count].content[sizeof(files[file_count].content)-1] = '\0';
  strncpy(files[file_count].owner, get_current_user(), sizeof(files[file_count].owner)-1);
  files[file_count].owner[sizeof(files[file_count].owner)-1] = '\0';
  files[file_count].shared = 0;
  files[file_count].is_dir = 0;
  file_count++;
  printf("File '%s' created and written (owner: %s).\n", name, get_current_user());
  save_filesystem();
}

void read_file(const char *name) {
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, name) == 0) {
      const char *cur = get_current_user();
      if (files[i].is_dir) {
        printf("'%s' is a directory. Use ls to view contents.\n", name);
        return;
      }
      if (!files[i].shared && strcmp(files[i].owner, cur) != 0 && !is_admin()) {
        printf("Permission denied: '%s' is private (owner: %s).\n", name, files[i].owner);
        return;
      }
      printf("Content of '%s': %s\n", name, files[i].content);
      return;
    }
  }
  printf("File not found.\n");
}

void delete_file(const char *name) {
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, name) == 0) {
      if (files[i].is_dir) {
        printf("'%s' is a directory. Use rmdir to remove directories.\n", name);
        return;
      }
      const char *cur = get_current_user();
      if (strcmp(files[i].owner, cur) != 0 && !is_admin()) {
        printf("Permission denied: only owner (%s) or admin can delete '%s'.\n", files[i].owner, name);
        return;
      }
      for (int j = i; j < file_count - 1; j++) {
        files[j] = files[j + 1];
      }
      file_count--;
      printf("File '%s' deleted.\n", name);
      save_filesystem();
      return;
    }
  }
  printf("File '%s' not found.\n", name);
}

void rename_file(const char *old_name, const char *new_name) {
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, old_name) == 0) {
      const char *cur = get_current_user();
      if (strcmp(files[i].owner, cur) != 0 && !is_admin()) {
        printf("Permission denied: only owner (%s) or admin can rename '%s'.\n", files[i].owner, old_name);
        return;
      }
      strcpy(files[i].name, new_name);
      printf("File renamed to '%s'.\n", new_name);
      save_filesystem();
      return;
    }
  }
  printf("File '%s' not found.\n", old_name);
}

void list_files() {
  printf("----------------------------------------------------------------------------------------------------\n");
  printf(" %-4s | %-20s | %-6s | %-6s | %-10s | %-6s \n", "ID", "Filename", "Type", "Size", "Owner", "Shared");
  printf("----------------------------------------------------------------------------------------------------\n");
  if (file_count == 0) {
    printf(" (No files)\n");
  } else {
    const char *cur = get_current_user();
    int shown = 0;
    for (int i = 0; i < file_count; i++) {
      int visible = is_admin() || files[i].shared || (strcmp(files[i].owner, cur) == 0);
      if (!visible) continue;
      shown++;
      printf(" %-4d | %-20s | %-6s | %-6lu | %-10s | %-6s\n", shown,
             files[i].name,
             files[i].is_dir ? "FOLDER" : "FILE",
             (unsigned long)strlen(files[i].content), files[i].owner,
             files[i].shared ? "yes" : "no");
    }
    if (!shown) printf(" (No visible files)\n");
  }
  printf("----------------------------------------------------------------------------------------------------\n");
}

void make_dir(const char *name) {
  if (file_count >= 50) {
    printf("Filesystem full.\n");
    return;
  }
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, name) == 0) {
      printf("A file or directory named '%s' already exists.\n", name);
      return;
    }
  }
  strcpy(files[file_count].name, name);
  files[file_count].content[0] = '\0';
  strncpy(files[file_count].owner, get_current_user(), sizeof(files[file_count].owner)-1);
  files[file_count].owner[sizeof(files[file_count].owner)-1] = '\0';
  files[file_count].shared = 0;
  files[file_count].is_dir = 1;
  file_count++;
  printf("Directory '%s' created (owner: %s).\n", name, get_current_user());
  save_filesystem();
}

void remove_dir(const char *name) {
  int idx = -1;
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, name) == 0) {
      idx = i;
      break;
    }
  }
  if (idx == -1) {
    printf("Directory '%s' not found.\n", name);
    return;
  }
  if (!files[idx].is_dir) {
    printf("'%s' is not a directory.\n", name);
    return;
  }
  const char *cur = get_current_user();
  if (strcmp(files[idx].owner, cur) != 0 && !is_admin()) {
    printf("Permission denied: only owner (%s) or admin can remove directory '%s'.\n", files[idx].owner, name);
    return;
  }
  size_t nlen = strlen(name);
  for (int i = 0; i < file_count; i++) {
    if (i == idx) continue;
    if (!files[i].is_dir) {
      if (strncmp(files[i].name, name, nlen) == 0 && files[i].name[nlen] == '/') {
        printf("Directory '%s' not empty.\n", name);
        return;
      }
    }
  }
  for (int j = idx; j < file_count - 1; j++) files[j] = files[j + 1];
  file_count--;
  printf("Directory '%s' removed.\n", name);
  save_filesystem();
}

void set_file_share(const char *name, int enabled) {
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, name) == 0) {
      const char *cur = get_current_user();
      if (strcmp(files[i].owner, cur) != 0 && !is_admin()) {
        printf("Permission denied: only owner (%s) or admin can change share status.\n", files[i].owner);
        return;
      }
      files[i].shared = enabled ? 1 : 0;
      printf("File '%s' share set to %s.\n", name, files[i].shared ? "enabled" : "disabled");
      save_filesystem();
      return;
    }
  }
  printf("File '%s' not found.\n", name);
}
