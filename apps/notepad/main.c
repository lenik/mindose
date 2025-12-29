#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../gui/gui.h"
#include "../../resource/resource.h"
#include "../../kernel/kernel.h"

// Notepad icon data (16x16, 4-bit color)
static const uint8_t notepad_icon_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x0F, 0xFF, 0xFF, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0x11, 0x11, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0x11, 0x11, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0x11, 0x11, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0x11, 0x11, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0x11, 0x11, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Resource section for this application
static resource_entry_t notepad_resources[] = {
    {ICON_NOTEPAD, RESOURCE_ICON, sizeof(notepad_icon_data), (void*)notepad_icon_data}
};

static resource_section_t notepad_resource_section = {
    .count = 1,
    .entries = notepad_resources
};

// Application state
typedef struct {
    window_t* window;
    widget_t* text_area;
    widget_t* btn_new;
    widget_t* btn_open;
    widget_t* btn_save;
    widget_t* btn_exit;
    char* text_buffer;
    size_t text_length;
    int is_modified;
    int running;
} notepad_app_t;

static notepad_app_t app_state = {0};

#define NOTEPAD_MAX_TEXT_SIZE 65536

void notepad_button_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget || !widget->text) return;
    
    if (strcmp(widget->text, "New") == 0) {
        // Clear text buffer
        if (app_state.text_buffer) {
            app_state.text_buffer[0] = '\0';
            app_state.text_length = 0;
            app_state.is_modified = 0;
            
            if (app_state.text_area && app_state.text_area->text) {
                free(app_state.text_area->text);
                app_state.text_area->text = strdup("");
            }
            
            strcpy(app_state.window->title, "Notepad - Untitled");
        }
        printf("Notepad: New file created\n");
    } else if (strcmp(widget->text, "Open") == 0) {
        // Simulate file opening
        const char* sample_text = "Sample text file content.\nThis is Notepad running as a standalone application!";
        
        if (strlen(sample_text) < NOTEPAD_MAX_TEXT_SIZE) {
            strcpy(app_state.text_buffer, sample_text);
            app_state.text_length = strlen(sample_text);
            app_state.is_modified = 0;
            
            if (app_state.text_area) {
                if (app_state.text_area->text) free(app_state.text_area->text);
                app_state.text_area->text = strdup(sample_text);
            }
            
            strcpy(app_state.window->title, "Notepad - document.txt");
        }
        printf("Notepad: File opened\n");
    } else if (strcmp(widget->text, "Save") == 0) {
        printf("Notepad: File saved (%zu bytes)\n", app_state.text_length);
        app_state.is_modified = 0;
    } else if (strcmp(widget->text, "Exit") == 0) {
        app_state.running = 0;
    }
}

int notepad_init(void) {
    printf("Notepad App: Initializing standalone application...\n");
    
    // Load resources
    resource_init();
    resource_load_from_section(&notepad_resource_section);
    
    // Create main window
    app_state.window = gui_create_window("Notepad - Untitled", 100, 100, 500, 400);
    if (!app_state.window) return -1;
    
    // Create menu buttons
    app_state.btn_new = gui_create_button("New", 10, 10, 60, 25);
    app_state.btn_open = gui_create_button("Open", 80, 10, 60, 25);
    app_state.btn_save = gui_create_button("Save", 150, 10, 60, 25);
    app_state.btn_exit = gui_create_button("Exit", 220, 10, 60, 25);
    
    // Create text area
    app_state.text_area = gui_create_textbox(10, 45, 470, 320);
    
    // Set up event handlers
    if (app_state.btn_new) app_state.btn_new->on_click = notepad_button_click;
    if (app_state.btn_open) app_state.btn_open->on_click = notepad_button_click;
    if (app_state.btn_save) app_state.btn_save->on_click = notepad_button_click;
    if (app_state.btn_exit) app_state.btn_exit->on_click = notepad_button_click;
    
    // Add widgets to window
    if (app_state.btn_new) gui_add_child_widget(&app_state.window->base, app_state.btn_new);
    if (app_state.btn_open) gui_add_child_widget(&app_state.window->base, app_state.btn_open);
    if (app_state.btn_save) gui_add_child_widget(&app_state.window->base, app_state.btn_save);
    if (app_state.btn_exit) gui_add_child_widget(&app_state.window->base, app_state.btn_exit);
    if (app_state.text_area) gui_add_child_widget(&app_state.window->base, app_state.text_area);
    
    // Initialize text buffer
    app_state.text_buffer = malloc(NOTEPAD_MAX_TEXT_SIZE);
    if (!app_state.text_buffer) return -1;
    
    app_state.text_buffer[0] = '\0';
    app_state.text_length = 0;
    app_state.is_modified = 0;
    app_state.running = 1;
    
    gui_show_window(app_state.window);
    
    printf("Notepad App: Initialized successfully\n");
    return 0;
}

void notepad_cleanup(void) {
    if (app_state.text_buffer) {
        free(app_state.text_buffer);
        app_state.text_buffer = NULL;
    }
    
    if (app_state.window) {
        gui_destroy_window(app_state.window);
        app_state.window = NULL;
    }
    
    resource_cleanup();
    printf("Notepad App: Cleaned up\n");
}

void notepad_main_loop(void) {
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

// Main entry point for standalone Notepad application
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    printf("Starting Notepad as standalone application...\n");
    
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
    
    if (notepad_init() != 0) {
        fprintf(stderr, "Failed to initialize Notepad\n");
        gui_cleanup();
        return 1;
    }
    
    // Run main loop
    notepad_main_loop();
    
    // Cleanup
    notepad_cleanup();
    gui_cleanup();
    
    printf("Notepad application terminated\n");
    return 0;
}
