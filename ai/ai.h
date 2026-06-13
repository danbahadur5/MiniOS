#ifndef AI_H
#define AI_H

void init_ai();
int ai_ask(const char *prompt, char *response, int max_len);
int ai_summarize(const char *text, char *summary, int max_len);
int ai_automate(const char *task, char *result, int max_len);
int ai_translate(const char *text, const char *lang, char *result, int max_len);
int ai_explain(const char *topic, char *result, int max_len);
int ai_code(const char *description, char *result, int max_len);
int ai_fix(const char *error, char *result, int max_len);
void ai_chat_reset();
int ai_chat_send(const char *message, char *response, int max_len);
const char* ai_get_model();
const char* ai_get_available_models();
int ai_set_model(const char *name);

#endif
