#ifndef FILESYSTEM_H
#define FILESYSTEM_H
void touch_file(const char *name);
void write_file(const char *name, const char *content);
void read_file(const char *name);
void delete_file(const char *name);
void rename_file(const char *old_name, const char *new_name);
void copy_file(const char *src, const char *dest);
void list_files();
void save_filesystem();
void load_filesystem();
#endif