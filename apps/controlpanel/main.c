#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "../../gui/gui.h"
#include "../../resource/resource.h"
#include "../../kernel/kernel.h"

// Control Panel icon data (16x16, 4-bit color)
static const uint8_t controlpanel_icon_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0x88, 0x88, 0xF0, 0x0F, 0x8F, 0xF8, 0xF0,
    0x0F, 0x8F, 0xF8, 0xF0, 0x0F, 0x88, 0x88, 0xF0,
    0x0F, 0x88, 0x88, 0xF0, 0x0F, 0x8F, 0xF8, 0xF0,
    0x0F, 0x8F, 0xF8, 0xF0, 0x0F, 0x88, 0x88, 0xF0,
    0x0F, 0x88, 0x88, 0xF0, 0x0F, 0x8F, 0xF8, 0xF0,
    0x0F, 0x8F, 0xF8, 0xF0, 0x0F, 0xFF, 0xFF, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Resource section for this application
static resource_entry_t controlpanel_resources[] = {
    {ICON_CONTROLPANEL, RESOURCE_ICON, sizeof(controlpanel_icon_data), (void*)controlpanel_icon_data}
};

static resource_section_t controlpanel_resource_section = {
    .count = 1,
    .entries = controlpanel_resources
};

// Control Panel state
typedef struct {
    window_t* window;
    widget_t* btn_display;
    widget_t* btn_mouse;
    widget_t* btn_keyboard;
    widget_t* btn_sound;
    widget_t* btn_system;
    widget_t* btn_about;
    widget_t* btn_exit;
    widget_t* info_label;
    int running;
} controlpanel_app_t;

static controlpanel_app_t app_state = {0};

void controlpanel_update_info(const char* text) {
    if (!app_state.info_label) return;
    
    if (app_state.info_label->text) {
        free(app_state.info_label->text);
    }
    app_state.info_label->text = strdup(text);
}

void controlpanel_button_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget || !widget->text) return;
    
    if (strcmp(widget->text, "Display") == 0) {
        controlpanel_update_info("Display Settings: Resolution, Colors, Screen Saver");
        printf("ControlPanel: Display settings selected\n");
    } else if (strcmp(widget->text, "Mouse") == 0) {
        controlpanel_update_info("Mouse Settings: Button configuration, Speed, Double-click");
        printf("ControlPanel: Mouse settings selected\n");
    } else if (strcmp(widget->text, "Keyboard") == 0) {
        controlpanel_update_info("Keyboard Settings: Repeat rate, Language, Layout");
        printf("ControlPanel: Keyboard settings selected\n");
    } else if (strcmp(widget->text, "Sound") == 0) {
        controlpanel_update_info("Sound Settings: Volume, Sound scheme, Audio devices");
        printf("ControlPanel: Sound settings selected\n");
    } else if (strcmp(widget->text, "System") == 0) {
        controlpanel_update_info("System Settings: Memory, Performance, Device Manager");
        printf("ControlPanel: System settings selected\n");
    } else if (strcmp(widget->text, "About") == 0) {
        controlpanel_update_info("Mindose OS v1.0 - Windows 3.1-like Operating System");
        printf("ControlPanel: About information displayed\n");
    } else if (strcmp(widget->text, "Exit") == 0) {
        app_state.running = 0;
    }
}

int controlpanel_init(void) {
    printf("ControlPanel App: Initializing standalone application...\n");
    
    // Load resources
    resource_init();
    resource_load_from_section(&controlpanel_resource_section);
    
    // Create main window
    app_state.window = gui_create_window("Control Panel", 150, 100, 400, 350);
    if (!app_state.window) return -1;
    
    // Create control panel icons/buttons in a grid layout
    app_state.btn_display = gui_create_button("Display", 20, 30, 80, 60);
    app_state.btn_mouse = gui_create_button("Mouse", 120, 30, 80, 60);
    app_state.btn_keyboard = gui_create_button("Keyboard", 220, 30, 80, 60);
    app_state.btn_sound = gui_create_button("Sound", 20, 110, 80, 60);
    app_state.btn_system = gui_create_button("System", 120, 110, 80, 60);
    app_state.btn_about = gui_create_button("About", 220, 110, 80, 60);
    
    // Create info display area
    app_state.info_label = gui_create_label("Select a control panel item to configure system settings", 
                                           20, 190, 360, 60);
    if (app_state.info_label) {
        app_state.info_label->bg_color = COLOR_WHITE;
        gui_add_child_widget(&app_state.window->base, app_state.info_label);
    }
    
    // Create exit button
    app_state.btn_exit = gui_create_button("Exit", 320, 270, 60, 25);
    
    // Set up button event handlers
    if (app_state.btn_display) {
        app_state.btn_display->on_click = controlpanel_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_display);
    }
    if (app_state.btn_mouse) {
        app_state.btn_mouse->on_click = controlpanel_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_mouse);
    }
    if (app_state.btn_keyboard) {
        app_state.btn_keyboard->on_click = controlpanel_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_keyboard);
    }
    if (app_state.btn_sound) {
        app_state.btn_sound->on_click = controlpanel_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_sound);
    }
    if (app_state.btn_system) {
        app_state.btn_system->on_click = controlpanel_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_system);
    }
    if (app_state.btn_about) {
        app_state.btn_about->on_click = controlpanel_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_about);
    }
    if (app_state.btn_exit) {
        app_state.btn_exit->on_click = controlpanel_button_click;
        gui_add_child_widget(&app_state.window->base, app_state.btn_exit);
    }
    
    // Initialize state
    app_state.running = 1;
    
    gui_show_window(app_state.window);
    
    printf("ControlPanel App: Initialized successfully\n");
    return 0;
}

void controlpanel_cleanup(void) {
    if (app_state.window) {
        gui_destroy_window(app_state.window);
        app_state.window = NULL;
    }
    
    resource_cleanup();
    printf("ControlPanel App: Cleaned up\n");
}

void controlpanel_main_loop(void) {
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

// Main entry point for standalone Control Panel application
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    printf("Starting Control Panel as standalone application...\n");
    
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
    
    if (controlpanel_init() != 0) {
        fprintf(stderr, "Failed to initialize Control Panel\n");
        gui_cleanup();
        return 1;
    }
    
    // Run main loop
    controlpanel_main_loop();
    
    // Cleanup
    controlpanel_cleanup();
    gui_cleanup();
    
    printf("Control Panel application terminated\n");
    return 0;
}
