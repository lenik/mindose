#ifndef APP_LOADER_H
#define APP_LOADER_H

#include <stdint.h>
#include <stddef.h>
#include "../common.h"

// Application information
typedef struct {
    char name[256];
    char executable_path[512];
    uint32_t icon_id;
    int is_running;
    uint32_t process_id;
} app_info_t;

// Application registry
typedef struct {
    app_info_t* apps;
    size_t app_count;
    size_t capacity;
} app_registry_t;

// Function declarations
int app_loader_init(void);
void app_loader_cleanup(void);

// Application management
int app_register(const char* name, const char* executable_path, uint32_t icon_id);
int app_launch(const char* name);
int app_terminate(const char* name);
app_info_t* app_find(const char* name);
void app_list_all(void);

// Built-in application registration
int app_register_builtin_apps(void);

// Process execution
uint32_t app_execute_standalone(const char* executable_path);
int app_wait_for_process(uint32_t process_id);

#endif // APP_LOADER_H
