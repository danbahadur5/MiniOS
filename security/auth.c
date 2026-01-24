#include "auth.h"
#include <stdio.h>
#include <string.h>
typedef struct {
  char username[20];
  char password[20];
  int is_admin;
} User;
User users[2] = {{"admin", "1234", 1}, {"user", "123", 0}};
int logged_in = -1;

void init_auth() { printf("Auth initialized.\n"); }
void login() {
  char uname[20], pwd[20];
  printf("Username: ");
  scanf("%s", uname);
  printf("Password: ");
  scanf("%s", pwd);
  for (int i = 0; i < 2; i++) {
    if (strcmp(uname, users[i].username) == 0 &&
        strcmp(pwd, users[i].password) == 0) {
      logged_in = i;
      printf("Login success! Welcome %s\n", uname);
      return;
    }
  }
  printf("Login failed!\n");
}
void logout() {
  if (logged_in != -1) {
    printf("User %s logged out.\n", users[logged_in].username);
    logged_in = -1;
  }
}
int is_admin() { return (logged_in != -1) ? users[logged_in].is_admin : 0; }
const char *get_current_user() {
  return (logged_in != -1) ? users[logged_in].username : "guest";
}