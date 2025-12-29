#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "../../gui/gui.h"
#include "../../resource/resource.h"
#include "../../kernel/kernel.h"

// Terminal icon data (16x16, 4-bit color)
static const uint8_t terminal_icon_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x0F, 0x00, 0xF0,
    0x0F, 0x0F, 0xF0, 0xF0, 0x0F, 0x0F, 0xFF, 0xF0,
    0x0F, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0xF0,
    0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x0F, 0x00, 0xF0,
    0x0F, 0x0F, 0x00, 0xF0, 0x0F, 0x00, 0x00, 0xF0,
    0x0F, 0x00, 0x00, 0xF0, 0x0F, 0xFF, 0xFF, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Resource section for this application
static resource_entry_t terminal_resources[] = {
    {ICON_TERMINAL, RESOURCE_ICON, sizeof(terminal_icon_data), (void*)terminal_icon_data}
};

static resource_section_t terminal_resource_section = {
    .count = 1,
    .entries = terminal_resources
};

#define MAX_OUTPUT_LINES 20
#define MAX_LINE_LENGTH 80
#define MAX_COMMAND_LENGTH 256

// Terminal state
typedef struct {
    window_t* window;
    widget_t* output_lines[MAX_OUTPUT_LINES];
    widget_t* input_field;
    widget_t* btn_clear;
    widget_t* btn_exit;
    char output_buffer[MAX_OUTPUT_LINES][MAX_LINE_LENGTH];
    char current_command[MAX_COMMAND_LENGTH];
    int output_line_count;
    int running;
} terminal_app_t;

static terminal_app_t app_state = {0};

void terminal_add_output_line(const char* text) {
    // Scroll up existing lines
    for (int i = 0; i < MAX_OUTPUT_LINES - 1; i++) {
        strcpy(app_state.output_buffer[i], app_state.output_buffer[i + 1]);
    }
    
    // Add new line at bottom
    strncpy(app_state.output_buffer[MAX_OUTPUT_LINES - 1], text, MAX_LINE_LENGTH - 1);
    app_state.output_buffer[MAX_OUTPUT_LINES - 1][MAX_LINE_LENGTH - 1] = '\0';
    
    if (app_state.output_line_count < MAX_OUTPUT_LINES) {
        app_state.output_line_count++;
    }
    
    // Update display
    for (int i = 0; i < MAX_OUTPUT_LINES; i++) {
        if (app_state.output_lines[i]) {
            if (app_state.output_lines[i]->text) {
                free(app_state.output_lines[i]->text);
            }
            app_state.output_lines[i]->text = strdup(app_state.output_buffer[i]);
        }
    }
}

void terminal_execute_command(const char* command) {
    char output_line[MAX_LINE_LENGTH];
    
    // Show command being executed
    snprintf(output_line, sizeof(output_line), "$ %s", command);
    terminal_add_output_line(output_line);
    
    // Handle built-in commands
    if (strcmp(command, "help") == 0) {
        terminal_add_output_line("Available commands:");
        terminal_add_output_line("  help    - Show this help");
        terminal_add_output_line("  clear   - Clear screen");
        terminal_add_output_line("  pwd     - Print working directory");
        terminal_add_output_line("  ls      - List directory contents");
        terminal_add_output_line("  date    - Show current date/time");
        terminal_add_output_line("  whoami  - Show current user");
        terminal_add_output_line("  exit    - Exit terminal");
    } else if (strcmp(command, "clear") == 0) {
        // Clear output buffer
        for (int i = 0; i < MAX_OUTPUT_LINES; i++) {
            strcpy(app_state.output_buffer[i], "");
        }
        app_state.output_line_count = 0;
        
        // Update display
        for (int i = 0; i < MAX_OUTPUT_LINES; i++) {
            if (app_state.output_lines[i]) {
                if (app_state.output_lines[i]->text) {
                    free(app_state.output_lines[i]->text);
                }
                app_state.output_lines[i]->text = strdup("");
            }
        }
    } else if (strcmp(command, "pwd") == 0) {
        char cwd[512];
        if (getcwd(cwd, sizeof(cwd))) {
            terminal_add_output_line(cwd);
        } else {
            terminal_add_output_line("Error: Cannot get current directory");
        }
    } else if (strcmp(command, "ls") == 0) {
        // Simple ls implementation
        FILE* fp = popen("ls -la", "r");
        if (fp) {
            char line[MAX_LINE_LENGTH];
            int line_count = 0;
            while (fgets(line, sizeof(line), fp) && line_count < 10) {
                // Remove newline
                line[strcspn(line, "\n")] = 0;
                terminal_add_output_line(line);
                line_count++;
            }
            pclose(fp);
        } else {
            terminal_add_output_line("Error: Cannot execute ls command");
        }
    } else if (strcmp(command, "date") == 0) {
        FILE* fp = popen("date", "r");
        if (fp) {
            char line[MAX_LINE_LENGTH];
            if (fgets(line, sizeof(line), fp)) {
                line[strcspn(line, "\n")] = 0;
                terminal_add_output_line(line);
            }
            pclose(fp);
        } else {
            terminal_add_output_line("Error: Cannot get date");
        }
    } else if (strcmp(command, "whoami") == 0) {
        FILE* fp = popen("whoami", "r");
        if (fp) {
            char line[MAX_LINE_LENGTH];
            if (fgets(line, sizeof(line), fp)) {
                line[strcspn(line, "\n")] = 0;
                terminal_add_output_line(line);
            }
            pclose(fp);
        } else {
            terminal_add_output_line("mindose");
        }
    } else if (strcmp(command, "exit") == 0) {
        app_state.running = 0;
    } else if (strlen(command) == 0) {
        // Empty command, do nothing
    } else {
        snprintf(output_line, sizeof(output_line), "Unknown command: %s (type 'help' for commands)", command);
        terminal_add_output_line(output_line);
    }
}

void terminal_input_handler(widget_t* widget, int key) {
    (void)widget;
    
    if (key == '\n' || key == '\r') {
        // Execute command
        terminal_execute_command(app_state.current_command);
        
        // Clear input
        strcpy(app_state.current_command, "");
        if (app_state.input_field && app_state.input_field->text) {
            free(app_state.input_field->text);
            app_state.input_field->text = strdup("");
        }
    } else if (key == '\b' || key == 127) {
        // Backspace
        int len = strlen(app_state.current_command);
        if (len > 0) {
            app_state.current_command[len - 1] = '\0';
            if (app_state.input_field && app_state.input_field->text) {
                free(app_state.input_field->text);
                app_state.input_field->text = strdup(app_state.current_command);
            }
        }
    } else if (key >= 32 && key < 127) {
        // Printable character
        int len = strlen(app_state.current_command);
        if (len < MAX_COMMAND_LENGTH - 1) {
            app_state.current_command[len] = key;
            app_state.current_command[len + 1] = '\0';
            if (app_state.input_field && app_state.input_field->text) {
                free(app_state.input_field->text);
                app_state.input_field->text = strdup(app_state.current_command);
            }
        }
    }
}

void terminal_button_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget || !widget->text) return;
    
    if (strcmp(widget->text, "Clear") == 0) {
        terminal_execute_command("clear");
    } else if (strcmp(widget->text, "Exit") == 0) {
        app_state.running = 0;
    }
}

int terminal_init(void) {
    printf("Terminal App: Initializing standalone application...\n");
    
    // Load resources
    resource_init();
    resource_load_from_section(&terminal_resource_section);
    
    // Create main window
    app_state.window = gui_create_window("Terminal", 100, 50, 600, 450);
    if (!app_state.window) return -1;
    
    // Create output display area
    for (int i = 0; i < MAX_OUTPUT_LINES; i++) {
        int y = 10 + i * 18;
        app_state.output_lines[i] = gui_create_label("", 10, y, 560, 15);
        if (app_state.output_lines[i]) {
            app_state.output_lines[i]->bg_color = COLOR_BLACK;
            app_state.output_lines[i]->fg_color = COLOR_GREEN;
            gui_add_child_widget(&app_state.window->base, app_state.output_lines[i]);
        }
        strcpy(app_state.output_buffer[i], "");
    }
    
    // Create input field (using label as input for now)
    app_state.input_field = gui_create_label("", 10, 380, 480, 25);
    if (app_state.input_field) {
        app_state.input_field->bg_color = COLOR_WHITE;
        app_state.input_field->fg_color = COLOR_BLACK;
        gui_add_child_widget(&app_state.window->base, app_state.input_field);
    }
    
    // Create control buttons
    app_state.btn_clear = gui_create_button("Clear", 500, 380, 60, 25);
    app_state.btn_exit = gui_create_button("Exit", 500, 410, 60, 25);
    
    // Set up button event handlers
    if (app_state.btn_clear) {
        app_state.btn_clear->on_click = terminal_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_clear);
    }
    if (app_state.btn_exit) {
        app_state.btn_exit->on_click = terminal_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_exit);
    }
    
    // Initialize state
    strcpy(app_state.current_command, "");
    app_state.output_line_count = 0;
    app_state.running = 1;
    
    // Show welcome message
    terminal_add_output_line("Mindose Terminal v1.0");
    terminal_add_output_line("Type 'help' for available commands");
    terminal_add_output_line("");
    
    gui_show_window(app_state.window);
    
    printf("Terminal App: Initialized successfully\n");
    return 0;
}

void terminal_cleanup(void) {
    if (app_state.window) {
        gui_destroy_window(app_state.window);
        app_state.window = NULL;
    }
    
    resource_cleanup();
    printf("Terminal App: Cleaned up\n");
}

void terminal_main_loop(void) {
    event_t event;
    
    while (app_state.running) {
        // Poll for events
        while (gui_poll_event(&event)) {
            gui_handle_event(&event);
            
            if (event.type == EVENT_WINDOW_CLOSE) {
                app_state.running = 0;
            } else if (event.type == EVENT_KEY_PRESS) {
                // Simple key handling - for now just handle basic commands
                terminal_input_handler(NULL, 'h'); // Placeholder
            }
        }
        
        // Redraw
        gui_refresh_screen();
        
        // Simulate frame rate
        usleep(50000); // 50ms = ~20 FPS
    }
}

// Main entry point for standalone Terminal application
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    printf("Starting Terminal as standalone application...\n");
    
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
    
    if (terminal_init() != 0) {
        fprintf(stderr, "Failed to initialize Terminal\n");
        gui_cleanup();
        return 1;
    }
    
    // Run main loop
    terminal_main_loop();
    
    // Cleanup
    terminal_cleanup();
    gui_cleanup();
    
    printf("Terminal application terminated\n");
    return 0;
}
