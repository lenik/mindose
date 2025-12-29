#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/stat.h>
#include "../../gui/gui.h"
#include "../../resource/resource.h"
#include "../../kernel/kernel.h"

// File Manager icon data (16x16, 4-bit color)
static const uint8_t filemanager_icon_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0x00, 0x0F, 0x44, 0x4F, 0xF0,
    0x0F, 0x44, 0x4F, 0xF0, 0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0x11, 0x11, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0x11, 0x11, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0x11, 0x11, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0x11, 0x11, 0xF0, 0x0F, 0xFF, 0xFF, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Resource section for this application
static resource_entry_t filemanager_resources[] = {
    {ICON_FILEMANAGER, RESOURCE_ICON, sizeof(filemanager_icon_data), (void*)filemanager_icon_data}
};

static resource_section_t filemanager_resource_section = {
    .count = 1,
    .entries = filemanager_resources
};

#define MAX_FILES 100
#define MAX_PATH_LEN 512

typedef struct {
    char name[256];
    int is_directory;
    long size;
} file_entry_t;

// File Manager state
typedef struct {
    window_t* window;
    widget_t* path_label;
    widget_t* file_list[MAX_FILES];
    widget_t* btn_up;
    widget_t* btn_refresh;
    widget_t* btn_exit;
    char current_path[MAX_PATH_LEN];
    file_entry_t files[MAX_FILES];
    int file_count;
    int running;
} filemanager_app_t;

static filemanager_app_t app_state = {0};

// Forward declarations
void filemanager_refresh_directory(void);

void filemanager_update_path_display(void) {
    if (!app_state.path_label) return;
    
    if (app_state.path_label->text) {
        free(app_state.path_label->text);
    }
    
    char display_path[MAX_PATH_LEN + 10];
    snprintf(display_path, sizeof(display_path), "Path: %s", app_state.current_path);
    app_state.path_label->text = strdup(display_path);
}

void filemanager_clear_file_list(void) {
    for (int i = 0; i < app_state.file_count; i++) {
        if (app_state.file_list[i]) {
            // Simply set to NULL since GUI system will handle cleanup
            app_state.file_list[i] = NULL;
        }
    }
    app_state.file_count = 0;
}

void filemanager_file_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget || !widget->text) return;
    
    // Find which file was clicked
    for (int i = 0; i < app_state.file_count; i++) {
        if (app_state.file_list[i] == widget) {
            if (app_state.files[i].is_directory) {
                // Navigate to directory
                if (strcmp(app_state.files[i].name, "..") == 0) {
                    // Go up one directory
                    char* last_slash = strrchr(app_state.current_path, '/');
                    if (last_slash && last_slash != app_state.current_path) {
                        *last_slash = '\0';
                    } else if (strcmp(app_state.current_path, "/") != 0) {
                        strcpy(app_state.current_path, "/");
                    }
                } else {
                    // Enter directory
                    if (strcmp(app_state.current_path, "/") != 0) {
                        strcat(app_state.current_path, "/");
                    }
                    strcat(app_state.current_path, app_state.files[i].name);
                }
                filemanager_refresh_directory();
            } else {
                printf("FileManager: Selected file: %s\n", app_state.files[i].name);
            }
            break;
        }
    }
}

void filemanager_refresh_directory(void) {
    DIR* dir = opendir(app_state.current_path);
    if (!dir) {
        printf("FileManager: Cannot open directory: %s\n", app_state.current_path);
        return;
    }
    
    // Clear existing file list
    filemanager_clear_file_list();
    
    // Read directory entries
    struct dirent* entry;
    app_state.file_count = 0;
    
    // Add parent directory entry if not at root
    if (strcmp(app_state.current_path, "/") != 0) {
        strcpy(app_state.files[app_state.file_count].name, "..");
        app_state.files[app_state.file_count].is_directory = 1;
        app_state.files[app_state.file_count].size = 0;
        app_state.file_count++;
    }
    
    while ((entry = readdir(dir)) != NULL && app_state.file_count < MAX_FILES - 1) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }
        
        strcpy(app_state.files[app_state.file_count].name, entry->d_name);
        
        // Get file stats
        char full_path[MAX_PATH_LEN];
        snprintf(full_path, sizeof(full_path), "%s/%s", app_state.current_path, entry->d_name);
        
        struct stat file_stat;
        if (stat(full_path, &file_stat) == 0) {
            app_state.files[app_state.file_count].is_directory = S_ISDIR(file_stat.st_mode);
            app_state.files[app_state.file_count].size = file_stat.st_size;
        } else {
            app_state.files[app_state.file_count].is_directory = 0;
            app_state.files[app_state.file_count].size = 0;
        }
        
        app_state.file_count++;
    }
    
    closedir(dir);
    
    // Create widgets for files
    for (int i = 0; i < app_state.file_count; i++) {
        int y = 70 + i * 25;
        
        char display_text[300];
        if (app_state.files[i].is_directory) {
            snprintf(display_text, sizeof(display_text), "[DIR] %s", app_state.files[i].name);
        } else {
            snprintf(display_text, sizeof(display_text), "%s (%ld bytes)", 
                    app_state.files[i].name, app_state.files[i].size);
        }
        
        app_state.file_list[i] = gui_create_button(display_text, 10, y, 460, 20);
        if (app_state.file_list[i]) {
            app_state.file_list[i]->on_click = filemanager_file_click;
            app_state.file_list[i]->bg_color = app_state.files[i].is_directory ? COLOR_LIGHT_GRAY : COLOR_WHITE;
            gui_add_child_widget(&app_state.window->base, app_state.file_list[i]);
        }
    }
    
    // Update path display
    filemanager_update_path_display();
    
    printf("FileManager: Loaded %d entries from %s\n", app_state.file_count, app_state.current_path);
}

void filemanager_button_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget || !widget->text) return;
    
    if (strcmp(widget->text, "Up") == 0) {
        char* last_slash = strrchr(app_state.current_path, '/');
        if (last_slash && last_slash != app_state.current_path) {
            *last_slash = '\0';
        } else if (strcmp(app_state.current_path, "/") != 0) {
            strcpy(app_state.current_path, "/");
        }
        filemanager_refresh_directory();
    } else if (strcmp(widget->text, "Refresh") == 0) {
        filemanager_refresh_directory();
    } else if (strcmp(widget->text, "Exit") == 0) {
        app_state.running = 0;
    }
}

int filemanager_init(void) {
    printf("FileManager App: Initializing standalone application...\n");
    
    // Load resources
    resource_init();
    resource_load_from_section(&filemanager_resource_section);
    
    // Create main window
    app_state.window = gui_create_window("File Manager", 50, 50, 500, 400);
    if (!app_state.window) return -1;
    
    // Create control buttons
    app_state.btn_up = gui_create_button("Up", 10, 10, 60, 25);
    app_state.btn_refresh = gui_create_button("Refresh", 80, 10, 80, 25);
    app_state.btn_exit = gui_create_button("Exit", 400, 10, 60, 25);
    
    // Create path label
    strcpy(app_state.current_path, "/home");
    app_state.path_label = gui_create_label("Path: /home", 10, 40, 400, 20);
    
    // Set up button event handlers
    if (app_state.btn_up) app_state.btn_up->on_click = filemanager_button_click;
    if (app_state.btn_refresh) app_state.btn_refresh->on_click = filemanager_button_click;
    if (app_state.btn_exit) app_state.btn_exit->on_click = filemanager_button_click;
    
    // Add control widgets to window
    if (app_state.btn_up) gui_add_child_widget(&app_state.window->base, app_state.btn_up);
    if (app_state.btn_refresh) gui_add_child_widget(&app_state.window->base, app_state.btn_refresh);
    if (app_state.btn_exit) gui_add_child_widget(&app_state.window->base, app_state.btn_exit);
    if (app_state.path_label) gui_add_child_widget(&app_state.window->base, app_state.path_label);
    
    // Initialize state
    app_state.file_count = 0;
    app_state.running = 1;
    
    // Load initial directory
    filemanager_refresh_directory();
    
    gui_show_window(app_state.window);
    
    printf("FileManager App: Initialized successfully\n");
    return 0;
}

void filemanager_cleanup(void) {
    filemanager_clear_file_list();
    
    if (app_state.window) {
        gui_destroy_window(app_state.window);
        app_state.window = NULL;
    }
    
    resource_cleanup();
    printf("FileManager App: Cleaned up\n");
}

void filemanager_main_loop(void) {
    event_t event;
    
    while (app_state.running) {
        // Poll for events
        while (gui_poll_event(&event)) {
            gui_handle_event(&event);
            
            if (event.type == EVENT_WINDOW_CLOSE) {
                app_state.running = 0;
            }
        }
        
        // Redraw
        gui_refresh_screen();
        
        // Simulate frame rate
        usleep(50000); // 50ms = ~20 FPS
    }
}

// Main entry point for standalone File Manager application
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    printf("Starting File Manager as standalone application...\n");
    
    // Initialize minimal GUI system for this app
    mindose_config_t config = {
        .mem_size = "64M",
        .diskimage = NULL,
        .iso = NULL,
        .arch = "x86",
        .application_mode = 1
    };
    
    if (gui_init(&config) != 0) {
        fprintf(stderr, "Failed to initialize GUI\n");
        return 1;
    }
    
    if (filemanager_init() != 0) {
        fprintf(stderr, "Failed to initialize File Manager\n");
        gui_cleanup();
        return 1;
    }
    
    // Run main loop
    filemanager_main_loop();
    
    // Cleanup
    filemanager_cleanup();
    gui_cleanup();
    
    printf("File Manager application terminated\n");
    return 0;
}
