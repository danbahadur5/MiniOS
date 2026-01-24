@echo off
echo Building MiniOS...
gcc -Wall main.c kernel/kernel.c cli/cli.c process/process.c scheduler/scheduler.c memory/memory.c filesystem/filesystem.c security/auth.c sync/sync.c monitor/monitor.c utils/logger.c -o MiniOS.exe
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b %errorlevel%
)
echo Build successful! Run MiniOS.exe to start.
