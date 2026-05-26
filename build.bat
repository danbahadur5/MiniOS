@echo off
echo Building MiniOS...
gcc -Wall main.c cli/cli.c process/process.c filesystem/filesystem.c security/auth.c monitor/monitor.c ai/ai.c -o MiniOS.exe -lws2_32
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)
echo Build successful! Run MiniOS.exe to start.
