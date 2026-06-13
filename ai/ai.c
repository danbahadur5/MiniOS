#include "ai.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#define OLLAMA_HOST "127.0.0.1"
#define OLLAMA_PORT 11434
#define BUF_SIZE 32768
#define DEFAULT_NUM_PREDICT 2048
#define DEFAULT_TEMPERATURE 0.7

static char ai_model[128] = "";
static char available_models[1024] = "";
static char chat_history[65536] = "";

void init_ai()
{
    printf("AI Module initializing...\n");

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("  WSAStartup failed\n");
        return;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        WSACleanup();
        return;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(OLLAMA_PORT);
    addr.sin_addr.s_addr = inet_addr(OLLAMA_HOST);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        WSACleanup();
        printf("  Ollama not found at %s:%d\n", OLLAMA_HOST, OLLAMA_PORT);
        printf("  Install Ollama from https://ollama.com and pull a model.\n");
        strncpy(ai_model, "llama3:latest", sizeof(ai_model) - 1);
        printf("  Defaulting to model: %s\n", ai_model);
        return;
    }

    char *req = "GET /api/tags HTTP/1.1\r\n"
                "Host: 127.0.0.1:11434\r\n"
                "Connection: close\r\n\r\n";
    send(sock, req, strlen(req), 0);

    char buf[BUF_SIZE];
    int total = 0, n;
    while (total < BUF_SIZE - 1 &&
           (n = recv(sock, buf + total, BUF_SIZE - 1 - total, 0)) > 0)
        total += n;
    buf[total] = '\0';
    closesocket(sock);
    WSACleanup();

    char *body = strstr(buf, "\r\n\r\n");
    if (!body)
    {
        printf("  Invalid response from Ollama\n");
        return;
    }
    body += 4;

    const char *search = "\"name\":\"";
    const char *p = body;
    int count = 0, avail_len = 0;

    while ((p = strstr(p, search)) != NULL &&
           avail_len < (int)sizeof(available_models) - 50)
    {
        p += strlen(search);
        char mname[128];
        int mi = 0;
        while (*p && *p != '"' && mi < 126)
            mname[mi++] = *p++;
        mname[mi] = '\0';

        if (count == 0)
            strncpy(ai_model, mname, sizeof(ai_model) - 1);

        if (avail_len > 0)
            avail_len += snprintf(available_models + avail_len,
                                  sizeof(available_models) - avail_len, ", ");
        avail_len += snprintf(available_models + avail_len,
                              sizeof(available_models) - avail_len, "%s", mname);
        count++;
    }

    if (count == 0)
    {
        printf("  No models found. Run: ollama pull llama3\n");
        strncpy(ai_model, "llama3:latest", sizeof(ai_model) - 1);
        snprintf(available_models, sizeof(available_models),
                 "llama3:latest (default)");
    }
    else
    {
        printf("  Connected to Ollama at %s:%d\n", OLLAMA_HOST, OLLAMA_PORT);
        printf("  Models: %s\n", available_models);
        printf("  Active model: %s\n", ai_model);
    }
}

const char *ai_get_model()
{
    return ai_model;
}

const char *ai_get_available_models()
{
    return available_models;
}

int ai_set_model(const char *name)
{
    if (strlen(name) >= sizeof(ai_model))
        return 0;
    strncpy(ai_model, name, sizeof(ai_model) - 1);
    ai_model[sizeof(ai_model) - 1] = '\0';
    return 1;
}

static void escape_json_string(const char *in, char *out, int out_len)
{
    int j = 0;
    for (int i = 0; in[i] && j < out_len - 4; i++)
    {
        char c = in[i];
        switch (c)
        {
        case '"':
            if (j < out_len - 2)
                out[j++] = '\\';
            out[j++] = '"';
            break;
        case '\\':
            if (j < out_len - 2)
                out[j++] = '\\';
            out[j++] = '\\';
            break;
        case '\n':
            if (j < out_len - 2)
                out[j++] = '\\';
            out[j++] = 'n';
            break;
        case '\r':
            if (j < out_len - 2)
                out[j++] = '\\';
            out[j++] = 'r';
            break;
        case '\t':
            if (j < out_len - 2)
                out[j++] = '\\';
            out[j++] = 't';
            break;
        default:
            out[j++] = c;
            break;
        }
    }
    out[j] = '\0';
}

static int extract_json_string(const char *json, const char *key,
                               char *out, int max_len)
{
    char search[128];
    snprintf(search, sizeof(search), "\"%s\":\"", key);
    const char *start = strstr(json, search);
    if (!start)
        return 0;
    start += strlen(search);
    int i = 0;
    while (*start && i < max_len - 1)
    {
        if (*start == '\\')
        {
            start++;
            if (*start == '"')
                out[i++] = '"';
            else if (*start == 'n')
                out[i++] = '\n';
            else if (*start == 'r')
                out[i++] = '\r';
            else if (*start == 't')
                out[i++] = '\t';
            else if (*start == '\\')
                out[i++] = '\\';
            else if (*start)
                out[i++] = *start;
            start++;
        }
        else if (*start == '"')
        {
            break;
        }
        else
        {
            out[i++] = *start++;
        }
    }
    out[i] = '\0';
    return i > 0;
}

static int req_ollama(const char *api_path, const char *json_body,
                      char *response, int max_len)
{
    if (strlen(ai_model) == 0)
        return 0;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
        return 0;

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET)
    {
        WSACleanup();
        return 0;
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(OLLAMA_PORT);
    addr.sin_addr.s_addr = inet_addr(OLLAMA_HOST);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) == SOCKET_ERROR)
    {
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    char request[BUF_SIZE];
    int body_len = strlen(json_body);
    int req_len = snprintf(request, sizeof(request),
                           "POST %s HTTP/1.1\r\n"
                           "Host: 127.0.0.1:11434\r\n"
                           "Content-Type: application/json\r\n"
                           "Content-Length: %d\r\n"
                           "Connection: close\r\n"
                           "\r\n"
                           "%s",
                           api_path, body_len, json_body);

    if (send(sock, request, req_len, 0) == SOCKET_ERROR)
    {
        closesocket(sock);
        WSACleanup();
        return 0;
    }

    char raw[BUF_SIZE];
    int total = 0, n;
    while (total < BUF_SIZE - 1 &&
           (n = recv(sock, raw + total, BUF_SIZE - 1 - total, 0)) > 0)
        total += n;
    raw[total] = '\0';
    closesocket(sock);
    WSACleanup();

    char *http_body = strstr(raw, "\r\n\r\n");
    if (!http_body)
        return 0;
    http_body += 4;

    char status[256];
    strncpy(status, raw, 255);
    status[255] = '\0';
    char *nl = strchr(status, '\r');
    if (nl)
        *nl = '\0';

    if (strstr(status, " 200 ") == NULL)
    {
        if (strstr(status, "404") != NULL)
            return 0;
        printf("  Ollama HTTP %s\n", status);
        return 0;
    }

    char tmp[BUF_SIZE];
    if (extract_json_string(http_body, "error", tmp, sizeof(tmp)))
    {
        printf("  Ollama: %s\n", tmp);
        return 0;
    }

    if (extract_json_string(http_body, "response", tmp, sizeof(tmp)))
    {
        strncpy(response, tmp, max_len - 1);
        response[max_len - 1] = '\0';
        return 1;
    }

    if (extract_json_string(http_body, "content", tmp, sizeof(tmp)))
    {
        strncpy(response, tmp, max_len - 1);
        response[max_len - 1] = '\0';
        return 1;
    }

    return 0;
}

static int send_prompt(const char *prompt, char *response, int max_len)
{
    char escaped[BUF_SIZE];
    escape_json_string(prompt, escaped, sizeof(escaped));

    printf("  Ollama generating (model: %s)...\n", ai_model);
    printf("  (This may take a while if the model needs to load)\n");

    char body[BUF_SIZE];
    snprintf(body, sizeof(body),
             "{\"model\":\"%s\",\"messages\":[{\"role\":\"user\","
             "\"content\":\"%s\"}],\"stream\":false,"
             "\"options\":{\"num_predict\":%d,\"temperature\":%.1f}}",
             ai_model, escaped, DEFAULT_NUM_PREDICT, DEFAULT_TEMPERATURE);

    if (req_ollama("/api/chat", body, response, max_len))
        return 1;

    snprintf(body, sizeof(body),
             "{\"model\":\"%s\",\"prompt\":\"%s\",\"stream\":false,"
             "\"options\":{\"num_predict\":%d,\"temperature\":%.1f}}",
             ai_model, escaped, DEFAULT_NUM_PREDICT, DEFAULT_TEMPERATURE);

    if (req_ollama("/api/generate", body, response, max_len))
        return 1;

    return 0;
}

int ai_ask(const char *prompt, char *response, int max_len)
{
    if (!send_prompt(prompt, response, max_len))
    {
        printf("  Error: Could not get response from model '%s'.\n"
               "  Make sure the model is pulled: ollama pull %s\n",
               ai_model, ai_model);
        return 0;
    }
    return 1;
}

int ai_summarize(const char *text, char *summary, int max_len)
{
    char prompt[4096];
    snprintf(prompt, sizeof(prompt),
             "Summarize the following text concisely in 2-3 sentences:\n\n%s", text);
    return ai_ask(prompt, summary, max_len);
}

int ai_automate(const char *task, char *result, int max_len)
{
    char prompt[4096];
    snprintf(prompt, sizeof(prompt),
             "You are an automation assistant for MiniOS (a command-line OS "
             "simulator). Given this task, suggest the exact MiniOS command(s) "
             "to run and briefly explain:\nTask: %s",
             task);
    return ai_ask(prompt, result, max_len);
}

int ai_translate(const char *text, const char *lang, char *result, int max_len)
{
    char prompt[4096];
    snprintf(prompt, sizeof(prompt),
             "Translate the following text to %s. Respond with only the translation, "
             "no explanations:\n\n%s",
             lang, text);
    return ai_ask(prompt, result, max_len);
}

int ai_explain(const char *topic, char *result, int max_len)
{
    char prompt[4096];
    snprintf(prompt, sizeof(prompt),
             "Explain the following concept briefly and clearly:\n\n%s", topic);
    return ai_ask(prompt, result, max_len);
}

int ai_code(const char *description, char *result, int max_len)
{
    char prompt[4096];
    snprintf(prompt, sizeof(prompt),
             "Write C code for the following. Provide only the code with brief "
             "comments, no extra explanation:\n\n%s",
             description);
    return ai_ask(prompt, result, max_len);
}

int ai_fix(const char *error, char *result, int max_len)
{
    char prompt[4096];
    snprintf(prompt, sizeof(prompt),
             "Explain what caused this error and how to fix it:\n\n%s", error);
    return ai_ask(prompt, result, max_len);
}

void ai_chat_reset()
{
    chat_history[0] = '\0';
    printf("  Chat history cleared.\n");
}

int ai_chat_send(const char *message, char *response, int max_len)
{
    if (*ai_model == '\0')
    {
        printf("  Error: No AI model available.\n");
        return 0;
    }

    char escaped_msg[BUF_SIZE];
    escape_json_string(message, escaped_msg, sizeof(escaped_msg));

    char user_entry[2048];
    snprintf(user_entry, sizeof(user_entry),
             "{\"role\":\"user\",\"content\":\"%s\"}", escaped_msg);

    char body[BUF_SIZE];
    if (chat_history[0] == '\0')
    {
        snprintf(body, sizeof(body),
                 "{\"model\":\"%s\",\"messages\":[%s],\"stream\":false,"
                 "\"options\":{\"num_predict\":%d,\"temperature\":%.1f}}",
                 ai_model, user_entry, DEFAULT_NUM_PREDICT, DEFAULT_TEMPERATURE);
    }
    else
    {
        snprintf(body, sizeof(body),
                 "{\"model\":\"%s\",\"messages\":[%s,%s],\"stream\":false,"
                 "\"options\":{\"num_predict\":%d,\"temperature\":%.1f}}",
                 ai_model, chat_history, user_entry, DEFAULT_NUM_PREDICT, DEFAULT_TEMPERATURE);
    }

    printf("  Ollama generating (model: %s)...\n", ai_model);

    if (!req_ollama("/api/chat", body, response, max_len))
    {
        snprintf(body, sizeof(body),
                 "{\"model\":\"%s\",\"prompt\":\"%s\",\"stream\":false,"
                 "\"options\":{\"num_predict\":%d,\"temperature\":%.1f}}",
                 ai_model, escaped_msg, DEFAULT_NUM_PREDICT, DEFAULT_TEMPERATURE);
        if (!req_ollama("/api/generate", body, response, max_len))
        {
            printf("  Error: Could not get response.\n");
            return 0;
        }
    }

    char escaped_resp[BUF_SIZE];
    escape_json_string(response, escaped_resp, sizeof(escaped_resp));

    char assistant_entry[2048];
    snprintf(assistant_entry, sizeof(assistant_entry),
             "{\"role\":\"assistant\",\"content\":\"%s\"}", escaped_resp);

    if (chat_history[0] == '\0')
    {
        snprintf(chat_history, sizeof(chat_history),
                 "%s,%s", user_entry, assistant_entry);
    }
    else
    {
        int cur = strlen(chat_history);
        if (cur < (int)sizeof(chat_history) - 4096)
        {
            snprintf(chat_history + cur, sizeof(chat_history) - cur,
                     ",%s,%s", user_entry, assistant_entry);
        }
    }

    return 1;
}
