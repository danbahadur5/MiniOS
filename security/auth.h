#ifndef AUTH_H
#define AUTH_H
void init_auth();
void login();
void logout();
int is_admin();
const char *get_current_user();
#endif