#include "filesystem.h"
#include <stdio.h>
#include <string.h>

#define FS_FILENAME "minios_fs.dat"

typedef struct {
  char name[50];
  char content[200];
} File;
File files[50];
int file_count = 0;

void save_filesystem() {
    FILE *f = fopen(FS_FILENAME, "wb");
    if (f) {
        fwrite(&file_count, sizeof(int), 1, f);
        fwrite(files, sizeof(File), file_count, f);
        fclose(f);
        // printf("DEBUG: Filesystem saved.\n");
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
  strcpy(files[file_count].name, name);
  files[file_count].content[0] = '\0';
  file_count++;
  printf("File '%s' created.\n", name);
  save_filesystem();
}
void write_file(const char *name, const char *content) {
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, name) == 0) {
      strcpy(files[i].content, content);
      printf("Written to '%s'.\n", name);
      save_filesystem();
      return;
    }
  }
  printf("File not found.\n");
}
void read_file(const char *name) {
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, name) == 0) {
      printf("Content of '%s': %s\n", name, files[i].content);
      return;
    }
  }
  printf("File not found.\n");
}

void delete_file(const char *name) {
  for (int i = 0; i < file_count; i++) {
    if (strcmp(files[i].name, name) == 0) {
      // Shift remaining files
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
      strcpy(files[i].name, new_name);
      printf("File renamed to '%s'.\n", new_name);
      save_filesystem();
      return;
    }
  }
  printf("File '%s' not found.\n", old_name);
}

void copy_file(const char *src, const char *dest) {
    int src_index = -1;
    // Find source file
    for(int i=0; i<file_count; i++){
        if(strcmp(files[i].name, src) == 0){
            src_index = i;
            break;
        }
    }
    
    if(src_index == -1){
        printf("Source file '%s' not found.\n", src);
        return;
    }
    
    // Check if dest exists
    for(int i=0; i<file_count; i++){
        if(strcmp(files[i].name, dest) == 0){
             printf("Destination file '%s' already exists.\n", dest);
             return;
        }
    }

    if(file_count >= 50) {
        printf("Filesystem full.\n");
        return;
    }

    // Copy data
    strcpy(files[file_count].name, dest);
    strcpy(files[file_count].content, files[src_index].content);
    file_count++;
    printf("File copied from '%s' to '%s'.\n", src, dest);
    save_filesystem();
}

void list_files() {
  printf("--------------------------------------------------\n");
  printf(" %-4s | %-20s | %-10s \n", "ID", "Filename", "Size");
  printf("--------------------------------------------------\n");
  if (file_count == 0) {
    printf(" (No files)\n");
  } else {
    for (int i = 0; i < file_count; i++) {
      printf(" %-4d | %-20s | %lu bytes\n", i + 1, files[i].name,
             (unsigned long)strlen(files[i].content));
    }
  }
  printf("--------------------------------------------------\n");
}