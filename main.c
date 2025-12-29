#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include "common.h"
#include "kernel/kernel.h"
#include "gui/gui.h"
#include "fs/filesystem.h"
#include "resource/resource.h"
#include "process/app_loader.h"

void print_usage(const char* program_name) {
    printf("Mindose - A Portable, Windows 3.1-like Operating System\n\n");
    printf("Usage: %s [OPTIONS]\n\n", program_name);
    printf("Options:\n");
    printf("  --mem SIZE        Specify memory size (e.g., 512M, 1G)\n");
    printf("  --diskimage FILE  Mount disk image file\n");
    printf("  --iso FILE        Mount ISO file as CD/DVD\n");
    printf("  --arch ARCH       Target architecture (x86, arm)\n");
    printf("  --help           Show this help message\n\n");
    printf("Examples:\n");
    printf("  %s --mem 512M --diskimage disk.img\n", program_name);
    printf("  %s --arch x86 --mem 1G --iso livecd.iso\n", program_name);
}

int parse_arguments(int argc, char* argv[], mindose_config_t* config) {
    static struct option long_options[] = {
        {"mem", required_argument, 0, 'm'},
        {"diskimage", required_argument, 0, 'd'},
        {"iso", required_argument, 0, 'i'},
        {"arch", required_argument, 0, 'a'},
        {"help", no_argument, 0, 'h'},
        {0, 0, 0, 0}
    };

    int opt;
    int option_index = 0;

    // Initialize config with defaults
    memset(config, 0, sizeof(mindose_config_t));
    config->mem_size = "256M";  // Default memory size
    config->arch = "x86";       // Default architecture
    config->application_mode = 1; // Default to application mode

    while ((opt = getopt_long(argc, argv, "m:d:i:a:h", long_options, &option_index)) != -1) {
        switch (opt) {
            case 'm':
                config->mem_size = strdup(optarg);
                break;
            case 'd':
                config->diskimage = strdup(optarg);
                break;
            case 'i':
                config->iso = strdup(optarg);
                break;
            case 'a':
                config->arch = strdup(optarg);
                break;
            case 'h':
                print_usage(argv[0]);
                return 0;
            default:
                print_usage(argv[0]);
                return -1;
        }
    }

    return 1;
}

int main(int argc, char* argv[]) {
    mindose_config_t config;
    
    printf("Mindose OS v1.0 - Starting...\n");
    
    // Parse command line arguments
    int parse_result = parse_arguments(argc, argv, &config);
    if (parse_result <= 0) {
        return parse_result == 0 ? 0 : 1;
    }

    // Initialize kernel
    printf("Initializing kernel...\n");
    if (kernel_init(&config) != 0) {
        fprintf(stderr, "Failed to initialize kernel\n");
        return 1;
    }

    // Initialize file system
    printf("Setting up file system...\n");
    if (filesystem_init(&config) != 0) {
        fprintf(stderr, "Failed to initialize file system\n");
        return 1;
    }

    // Initialize GUI system
    printf("Starting GUI system...\n");
    if (gui_init(&config) != 0) {
        fprintf(stderr, "Failed to initialize GUI\n");
        return 1;
    }

    // Initialize resource system
    printf("Initializing resource system...\n");
    if (resource_init() != 0) {
        fprintf(stderr, "Failed to initialize resource system\n");
        return 1;
    }

    // Initialize application loader
    printf("Initializing application loader...\n");
    if (app_loader_init() != 0) {
        fprintf(stderr, "Failed to initialize application loader\n");
        return 1;
    }

    // Register built-in applications
    printf("Registering built-in applications...\n");
    app_register_builtin_apps();
    app_list_all();

    // Main event loop
    printf("Mindose OS ready!\n");
    gui_main_loop();

    // Cleanup systems
    app_loader_cleanup();
    resource_cleanup();

    // Cleanup core systems
    gui_cleanup();
    filesystem_cleanup();
    kernel_cleanup();

    return 0;
}
