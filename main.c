#include <stdio.h>
#include <windows.h>
#include "ai/ai.h"
#include "cli/cli.h"
#include "process/process.h"
#include "filesystem/filesystem.h"
#include "security/auth.h"
#include "monitor/monitor.h"

void boot_animation()
{
    printf("Booting MiniOS v1.0...\n");
    printf("[");
    for (int i = 0; i <= 50; i++)
    {
        printf("=");
        Sleep(30);
    }
    printf("] 100%%\n");
    printf("Loading Kernel...");
    Sleep(500);
    printf(" Done.\n");
    printf("Mounting Filesystem...");
    Sleep(500);
    printf(" Done.\n");
    printf("Starting Services...");
    Sleep(500);
    printf(" Done.\n");
    printf("Loading AI Module...");
    Sleep(500);
    printf(" Done.\n");
    Sleep(500);
}

int main()
{
    boot_animation();
    load_filesystem();
    init_auth();
    init_system_monitor();
    init_ai();
    start_cli();
    return 0;
}
