#ifndef AI_H
#define AI_H

void init_ai();
int ai_ask(const char *prompt, char *response, int max_len);
int ai_summarize(const char *text, char *summary, int max_len);
int ai_automate(const char *task, char *result, int max_len);
const char* ai_get_model();
const char* ai_get_available_models();
int ai_set_model(const char *name);

#endif
