@echo off
echo Building MiniOS...
cmd /c del /f /q MiniOS.exe
gcc -Wall main.c cli/cli.c process/process.c filesystem/filesystem.c security/auth.c monitor/monitor.c ai/ai.c -o MiniOS.exe -lws2_32 -lshell32

echo Build successful! Run MiniOS.exe to start.
