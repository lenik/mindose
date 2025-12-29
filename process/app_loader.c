#define _GNU_SOURCE
#include "app_loader.h"
#include "../kernel/kernel.h"
#include "../resource/resource.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

static app_registry_t app_registry = {0};

int app_loader_init(void) {
    app_registry.app_count = 0;
    app_registry.capacity = 10; // Initial capacity
    app_registry.apps = malloc(sizeof(app_info_t) * app_registry.capacity);
    
    if (!app_registry.apps) {
        return -1;
    }
    
    printf("AppLoader: Initialized\n");
    return 0;
}

void app_loader_cleanup(void) {
    if (app_registry.apps) {
        free(app_registry.apps);
        app_registry.apps = NULL;
    }
    app_registry.app_count = 0;
    app_registry.capacity = 0;
    printf("AppLoader: Cleaned up\n");
}

int app_register(const char* name, const char* executable_path, uint32_t icon_id) {
    if (!name || !executable_path) return -1;
    
    // Check if app already exists
    for (size_t i = 0; i < app_registry.app_count; i++) {
        if (strcmp(app_registry.apps[i].name, name) == 0) {
            // Update existing app
            strncpy(app_registry.apps[i].executable_path, executable_path, 
                   sizeof(app_registry.apps[i].executable_path) - 1);
            app_registry.apps[i].executable_path[sizeof(app_registry.apps[i].executable_path) - 1] = '\0';
            app_registry.apps[i].icon_id = icon_id;
            return 0;
        }
    }
    
    // Expand registry if needed
    if (app_registry.app_count >= app_registry.capacity) {
        size_t new_capacity = app_registry.capacity * 2;
        app_info_t* new_apps = realloc(app_registry.apps, sizeof(app_info_t) * new_capacity);
        if (!new_apps) return -1;
        
        app_registry.apps = new_apps;
        app_registry.capacity = new_capacity;
    }
    
    // Add new app
    app_info_t* app = &app_registry.apps[app_registry.app_count];
    strncpy(app->name, name, sizeof(app->name) - 1);
    app->name[sizeof(app->name) - 1] = '\0';
    strncpy(app->executable_path, executable_path, sizeof(app->executable_path) - 1);
    app->executable_path[sizeof(app->executable_path) - 1] = '\0';
    app->icon_id = icon_id;
    app->is_running = 0;
    app->process_id = 0;
    
    app_registry.app_count++;
    
    printf("AppLoader: Registered application '%s' -> '%s'\n", name, executable_path);
    return 0;
}

app_info_t* app_find(const char* name) {
    if (!name) return NULL;
    
    for (size_t i = 0; i < app_registry.app_count; i++) {
        if (strcmp(app_registry.apps[i].name, name) == 0) {
            return &app_registry.apps[i];
        }
    }
    
    return NULL;
}

uint32_t app_execute_standalone(const char* executable_path) {
    if (!executable_path) return 0;
    
    printf("AppLoader: Executing standalone app: %s\n", executable_path);
    
    // Fork a new process to run the standalone application
    pid_t pid = fork();
    
    if (pid == 0) {
        // Child process - execute the application
        execl(executable_path, executable_path, NULL);
        
        // If execl returns, there was an error
        fprintf(stderr, "AppLoader: Failed to execute %s\n", executable_path);
        exit(1);
    } else if (pid > 0) {
        // Parent process - return the process ID
        printf("AppLoader: Started process %d for %s\n", pid, executable_path);
        return (uint32_t)pid;
    } else {
        // Fork failed
        fprintf(stderr, "AppLoader: Failed to fork process for %s\n", executable_path);
        return 0;
    }
}

int app_launch(const char* name) {
    app_info_t* app = app_find(name);
    if (!app) {
        printf("AppLoader: Application '%s' not found\n", name);
        return -1;
    }
    
    if (app->is_running) {
        printf("AppLoader: Application '%s' is already running (PID: %u)\n", name, app->process_id);
        return 0;
    }
    
    // Check if executable exists
    if (access(app->executable_path, F_OK) != 0) {
        printf("AppLoader: Executable not found: %s\n", app->executable_path);
        return -1;
    }
    
    // Launch the application
    uint32_t pid = app_execute_standalone(app->executable_path);
    if (pid == 0) {
        return -1;
    }
    
    app->process_id = pid;
    app->is_running = 1;
    
    printf("AppLoader: Launched '%s' (PID: %u)\n", name, pid);
    return 0;
}

int app_terminate(const char* name) {
    app_info_t* app = app_find(name);
    if (!app) {
        return -1;
    }
    
    if (!app->is_running) {
        return 0;
    }
    
    // Terminate the process
    if (kill(app->process_id, SIGTERM) == 0) {
        app->is_running = 0;
        app->process_id = 0;
        printf("AppLoader: Terminated '%s'\n", name);
        return 0;
    }
    
    return -1;
}

int app_wait_for_process(uint32_t process_id) {
    int status;
    pid_t result = waitpid((pid_t)process_id, &status, WNOHANG);
    
    if (result == (pid_t)process_id) {
        // Process has terminated
        printf("AppLoader: Process %u terminated\n", process_id);
        
        // Update app registry
        for (size_t i = 0; i < app_registry.app_count; i++) {
            if (app_registry.apps[i].process_id == process_id) {
                app_registry.apps[i].is_running = 0;
                app_registry.apps[i].process_id = 0;
                break;
            }
        }
        
        return 1; // Process terminated
    } else if (result == 0) {
        return 0; // Process still running
    } else {
        return -1; // Error
    }
}

void app_list_all(void) {
    printf("AppLoader: Registered applications:\n");
    for (size_t i = 0; i < app_registry.app_count; i++) {
        const app_info_t* app = &app_registry.apps[i];
        printf("  %s -> %s (Icon: %u, Running: %s)\n", 
               app->name, app->executable_path, app->icon_id,
               app->is_running ? "Yes" : "No");
    }
}

int app_register_builtin_apps(void) {
    // Register built-in applications
    int result = 0;
    
    result |= app_register("Notepad", "./notepad_app", ICON_NOTEPAD);
    result |= app_register("Paint", "./paint_app", ICON_PAINT);
    result |= app_register("Minesweeper", "./minesweeper_app", ICON_MINESWEEPER);
    result |= app_register("Calculator", "./calculator_app", ICON_CALCULATOR);
    result |= app_register("File Manager", "./filemanager_app", ICON_FILEMANAGER);
    result |= app_register("Clock", "./clock_app", ICON_CLOCK);
    result |= app_register("Control Panel", "./controlpanel_app", ICON_CONTROLPANEL);
    result |= app_register("Terminal", "./terminal_app", ICON_TERMINAL);
    
    if (result == 0) {
        printf("AppLoader: Built-in applications registered\n");
    }
    
    return result;
}
