# MiniOS-AI

Command-line OS simulator written in C, with local AI capabilities via Ollama.

## How to Run

### 1. Build
Double-click `build.bat` or run in terminal:
```
build.bat
```

### 2. Run
```
gcc -Wall main.c cli/cli.c process/process.c filesystem/filesystem.c security/auth.c monitor/monitor.c ai/ai.c -o MiniOS.exe -lws2_32


MiniOS.exe
```

### 3. AI Features (optional)
Install [Ollama](https://ollama.com), pull a model:
```
ollama pull llama3
```
Then AI commands (`ai ask`, `ai summarize`, `ai automate`) will work inside MiniOS — no internet needed.

### Commands
- `help` — list all commands
- `login` — switch user (admin:admin123, user1:pass1, etc.)
- `shutdown` — exit
- `ai ask <question>` — ask the local AI
