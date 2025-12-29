#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include "../../gui/gui.h"
#include "../../resource/resource.h"
#include "../../kernel/kernel.h"

// Clock icon data (16x16, 4-bit color)
static const uint8_t clock_icon_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x0F, 0xF0, 0x00,
    0x00, 0xFF, 0xFF, 0x00, 0x0F, 0x11, 0x11, 0xF0,
    0x0F, 0x18, 0x81, 0xF0, 0x0F, 0x18, 0x81, 0xF0,
    0x0F, 0x18, 0x81, 0xF0, 0x0F, 0x18, 0x81, 0xF0,
    0x0F, 0x18, 0x81, 0xF0, 0x0F, 0x18, 0x81, 0xF0,
    0x0F, 0x18, 0x81, 0xF0, 0x0F, 0x11, 0x11, 0xF0,
    0x00, 0xFF, 0xFF, 0x00, 0x00, 0x0F, 0xF0, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Resource section for this application
static resource_entry_t clock_resources[] = {
    {ICON_CLOCK, RESOURCE_ICON, sizeof(clock_icon_data), (void*)clock_icon_data}
};

static resource_section_t clock_resource_section = {
    .count = 1,
    .entries = clock_resources
};

// Clock state
typedef struct {
    window_t* window;
    widget_t* time_display;
    widget_t* date_display;
    widget_t* btn_exit;
    int running;
} clock_app_t;

static clock_app_t app_state = {0};

void clock_update_display(void) {
    time_t now;
    struct tm* local_time;
    char time_str[64];
    char date_str[64];
    
    time(&now);
    local_time = localtime(&now);
    
    // Format time (12-hour format with AM/PM)
    strftime(time_str, sizeof(time_str), "%I:%M:%S %p", local_time);
    
    // Format date
    strftime(date_str, sizeof(date_str), "%A, %B %d, %Y", local_time);
    
    // Update time display
    if (app_state.time_display) {
        if (app_state.time_display->text) {
            free(app_state.time_display->text);
        }
        app_state.time_display->text = strdup(time_str);
    }
    
    // Update date display
    if (app_state.date_display) {
        if (app_state.date_display->text) {
            free(app_state.date_display->text);
        }
        app_state.date_display->text = strdup(date_str);
    }
}

void clock_button_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget || !widget->text) return;
    
    if (strcmp(widget->text, "Exit") == 0) {
        app_state.running = 0;
    }
}

int clock_init(void) {
    printf("Clock App: Initializing standalone application...\n");
    
    // Load resources
    resource_init();
    resource_load_from_section(&clock_resource_section);
    
    // Create main window
    app_state.window = gui_create_window("Clock", 200, 150, 300, 150);
    if (!app_state.window) return -1;
    
    // Create time display (large text)
    app_state.time_display = gui_create_label("00:00:00 AM", 20, 20, 260, 40);
    if (app_state.time_display) {
        app_state.time_display->bg_color = COLOR_BLACK;
        app_state.time_display->fg_color = COLOR_GREEN;
        gui_add_child_widget(&app_state.window->base, app_state.time_display);
    }
    
    // Create date display
    app_state.date_display = gui_create_label("Monday, January 01, 2024", 20, 70, 260, 20);
    if (app_state.date_display) {
        app_state.date_display->bg_color = COLOR_LIGHT_GRAY;
        gui_add_child_widget(&app_state.window->base, app_state.date_display);
    }
    
    // Create exit button
    app_state.btn_exit = gui_create_button("Exit", 220, 100, 60, 25);
    if (app_state.btn_exit) {
        app_state.btn_exit->on_click = clock_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_exit);
    }
    
    // Initialize state
    app_state.running = 1;
    
    // Update display initially
    clock_update_display();
    
    gui_show_window(app_state.window);
    
    printf("Clock App: Initialized successfully\n");
    return 0;
}

void clock_cleanup(void) {
    if (app_state.window) {
        gui_destroy_window(app_state.window);
        app_state.window = NULL;
    }
    
    resource_cleanup();
    printf("Clock App: Cleaned up\n");
}

void clock_main_loop(void) {
    event_t event;
    int frame_count = 0;
    
    while (app_state.running) {
        // Poll for events
        while (gui_poll_event(&event)) {
            gui_handle_event(&event);
            
            if (event.type == EVENT_WINDOW_CLOSE) {
                app_state.running = 0;
            }
        }
        
        // Update clock display every second (20 frames at 20 FPS)
        if (frame_count % 20 == 0) {
            clock_update_display();
        }
        frame_count++;
        
        // Redraw
        gui_refresh_screen();
        
        // Simulate frame rate
        usleep(50000); // 50ms = ~20 FPS
    }
}

// Main entry point for standalone Clock application
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    printf("Starting Clock as standalone application...\n");
    
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
    
    if (clock_init() != 0) {
        fprintf(stderr, "Failed to initialize Clock\n");
        gui_cleanup();
        return 1;
    }
    
    // Run main loop
    clock_main_loop();
    
    // Cleanup
    clock_cleanup();
    gui_cleanup();
    
    printf("Clock application terminated\n");
    return 0;
}
