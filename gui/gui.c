#define _GNU_SOURCE
#include "gui.h"
#include "../process/app_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static gui_manager_t gui_mgr = {0};

// Simple framebuffer simulation (in real implementation would use actual graphics)
static char screen_buffer[80 * 25]; // 80x25 text mode simulation
static color_t color_buffer[80 * 25];

int gui_init(mindose_config_t* config) {
    printf("GUI: Initializing Windows 3.1-like interface...\n");
    
    // Initialize screen dimensions
    gui_mgr.screen_width = 640;  // VGA resolution
    gui_mgr.screen_height = 480;
    gui_mgr.desktop_color = COLOR_DARK_CYAN;
    
    // Clear screen buffer
    memset(screen_buffer, ' ', sizeof(screen_buffer));
    for (int i = 0; i < 80 * 25; i++) {
        color_buffer[i] = gui_mgr.desktop_color;
    }
    
    // Create desktop widget
    gui_mgr.desktop = gui_create_widget(WIDGET_WINDOW, 0, 0, gui_mgr.screen_width, gui_mgr.screen_height);
    if (!gui_mgr.desktop) {
        return -1;
    }
    
    gui_mgr.desktop->bg_color = gui_mgr.desktop_color;
    gui_mgr.windows = NULL;
    gui_mgr.active_window = NULL;
    gui_mgr.initialized = 1;
    
    printf("GUI: Initialized (%dx%d)\n", gui_mgr.screen_width, gui_mgr.screen_height);
    return 0;
}

void gui_cleanup(void) {
    if (!gui_mgr.initialized) return;
    
    printf("GUI: Shutting down...\n");
    
    // Destroy all windows
    window_t* window = gui_mgr.windows;
    while (window) {
        window_t* next = window->next;
        gui_destroy_window(window);
        window = next;
    }
    
    // Destroy desktop
    if (gui_mgr.desktop) {
        gui_destroy_widget(gui_mgr.desktop);
        gui_mgr.desktop = NULL;
    }
    
    gui_mgr.initialized = 0;
}

void gui_main_loop(void) {
    if (!gui_mgr.initialized) return;
    
    printf("GUI: Starting main event loop...\n");
    
    // Create a sample window to demonstrate
    window_t* sample_window = gui_create_window("Mindose Desktop", 50, 50, 400, 300);
    if (sample_window) {
        gui_show_window(sample_window);
        
        // Add application launcher buttons in a grid layout
        const char* app_names[] = {
            "Notepad", "Paint", "Minesweeper", "Calculator",
            "File Manager", "Clock", "Control Panel", "Terminal"
        };
        
        widget_t* app_buttons[8];
        widget_t* label = gui_create_label("Welcome to Mindose!", 10, 20, 300, 20);
        
        // Create buttons in a 4x2 grid
        for (int i = 0; i < 8; i++) {
            int row = i / 4;
            int col = i % 4;
            int x = 10 + col * 95;
            int y = 50 + row * 35;
            int width = (i == 4) ? 110 : 90; // Make "File Manager" button wider
            
            app_buttons[i] = gui_create_button(app_names[i], x, y, width, 30);
            if (app_buttons[i]) {
                app_buttons[i]->on_click = gui_launch_app_handler;
                gui_add_child_widget(&sample_window->base, app_buttons[i]);
            }
        }
        
        if (label) gui_add_child_widget(&sample_window->base, label);
    }
    
    // Main event loop
    event_t event;
    int running = 1;
    int frame_count = 0;
    
    while (running) {
        // Poll for events
        while (gui_poll_event(&event)) {
            gui_handle_event(&event);
            
            if (event.type == EVENT_WINDOW_CLOSE) {
                running = 0;
            }
        }
        
        // Redraw screen
        gui_refresh_screen();
        
        // Simulate frame rate
        usleep(50000); // 50ms = ~20 FPS
        frame_count++;
        
        // Exit after demonstration
        if (frame_count > 100) { // Run for ~5 seconds
            printf("GUI: Demo complete, exiting main loop\n");
            running = 0;
        }
    }
}

window_t* gui_create_window(const char* title, int x, int y, int width, int height) {
    window_t* window = malloc(sizeof(window_t));
    if (!window) return NULL;
    
    // Initialize base widget
    window->base.type = WIDGET_WINDOW;
    window->base.bounds.x = x;
    window->base.bounds.y = y;
    window->base.bounds.width = width;
    window->base.bounds.height = height;
    window->base.text = NULL;
    window->base.bg_color = COLOR_LIGHT_GRAY;
    window->base.fg_color = COLOR_BLACK;
    window->base.visible = 0;
    window->base.enabled = 1;
    window->base.parent = NULL;
    window->base.children = NULL;
    window->base.next_sibling = NULL;
    window->base.on_click = NULL;
    window->base.on_paint = NULL;
    
    // Initialize window-specific properties
    strncpy(window->title, title ? title : "Untitled", sizeof(window->title) - 1);
    window->title[sizeof(window->title) - 1] = '\0';
    window->minimized = 0;
    window->maximized = 0;
    window->has_titlebar = 1;
    window->has_border = 1;
    window->resizable = 1;
    
    // Add to window list
    window->next = gui_mgr.windows;
    gui_mgr.windows = window;
    
    printf("GUI: Created window '%s' (%dx%d at %d,%d)\n", window->title, width, height, x, y);
    return window;
}

void gui_destroy_window(window_t* window) {
    if (!window) return;
    
    // Remove from window list
    if (gui_mgr.windows == window) {
        gui_mgr.windows = window->next;
    } else {
        window_t* current = gui_mgr.windows;
        while (current && current->next != window) {
            current = current->next;
        }
        if (current) {
            current->next = window->next;
        }
    }
    
    // Update active window if needed
    if (gui_mgr.active_window == window) {
        gui_mgr.active_window = gui_mgr.windows;
    }
    
    // Destroy child widgets
    gui_destroy_widget(&window->base);
    
    printf("GUI: Destroyed window '%s'\n", window->title);
    free(window);
}

void gui_show_window(window_t* window) {
    if (!window) return;
    window->base.visible = 1;
    gui_set_active_window(window);
}

void gui_hide_window(window_t* window) {
    if (!window) return;
    window->base.visible = 0;
}

void gui_set_active_window(window_t* window) {
    gui_mgr.active_window = window;
}

widget_t* gui_create_widget(widget_type_t type, int x, int y, int width, int height) {
    widget_t* widget = malloc(sizeof(widget_t));
    if (!widget) return NULL;
    
    widget->type = type;
    widget->bounds.x = x;
    widget->bounds.y = y;
    widget->bounds.width = width;
    widget->bounds.height = height;
    widget->text = NULL;
    widget->bg_color = COLOR_LIGHT_GRAY;
    widget->fg_color = COLOR_BLACK;
    widget->visible = 1;
    widget->enabled = 1;
    widget->parent = NULL;
    widget->children = NULL;
    widget->next_sibling = NULL;
    widget->on_click = NULL;
    widget->on_paint = NULL;
    
    return widget;
}

void gui_destroy_widget(widget_t* widget) {
    if (!widget) return;
    
    // Destroy children recursively
    widget_t* child = widget->children;
    while (child) {
        widget_t* next = child->next_sibling;
        gui_destroy_widget(child);
        child = next;
    }
    
    if (widget->text) {
        free(widget->text);
    }
    
    free(widget);
}

void gui_add_child_widget(widget_t* parent, widget_t* child) {
    if (!parent || !child) return;
    
    child->parent = parent;
    child->next_sibling = parent->children;
    parent->children = child;
}

widget_t* gui_create_button(const char* text, int x, int y, int width, int height) {
    widget_t* button = gui_create_widget(WIDGET_BUTTON, x, y, width, height);
    if (!button) return NULL;
    
    if (text) {
        button->text = strdup(text);
    }
    
    button->bg_color = COLOR_LIGHT_GRAY;
    button->fg_color = COLOR_BLACK;
    
    return button;
}

widget_t* gui_create_label(const char* text, int x, int y, int width, int height) {
    widget_t* label = gui_create_widget(WIDGET_LABEL, x, y, width, height);
    if (!label) return NULL;
    
    if (text) {
        label->text = strdup(text);
    }
    
    label->bg_color = COLOR_LIGHT_GRAY;
    label->fg_color = COLOR_BLACK;
    
    return label;
}

widget_t* gui_create_textbox(int x, int y, int width, int height) {
    widget_t* textbox = gui_create_widget(WIDGET_TEXTBOX, x, y, width, height);
    if (!textbox) return NULL;
    
    textbox->bg_color = COLOR_WHITE;
    textbox->fg_color = COLOR_BLACK;
    
    return textbox;
}

void gui_draw_rect(rect_t rect, color_t color) {
    // Simple text-mode rectangle drawing
    int start_x = rect.x / 8; // Convert to character coordinates
    int start_y = rect.y / 16;
    int end_x = (rect.x + rect.width) / 8;
    int end_y = (rect.y + rect.height) / 16;
    
    for (int y = start_y; y < end_y && y < 25; y++) {
        for (int x = start_x; x < end_x && x < 80; x++) {
            if (x >= 0 && y >= 0) {
                color_buffer[y * 80 + x] = color;
            }
        }
    }
}

void gui_draw_border(rect_t rect, color_t color) {
    // Draw border characters in text mode
    int start_x = rect.x / 8;
    int start_y = rect.y / 16;
    int end_x = (rect.x + rect.width) / 8;
    int end_y = (rect.y + rect.height) / 16;
    
    // Top and bottom borders
    for (int x = start_x; x < end_x && x < 80; x++) {
        if (x >= 0 && start_y >= 0 && start_y < 25) {
            screen_buffer[start_y * 80 + x] = '-';
            color_buffer[start_y * 80 + x] = color;
        }
        if (x >= 0 && end_y - 1 >= 0 && end_y - 1 < 25) {
            screen_buffer[(end_y - 1) * 80 + x] = '-';
            color_buffer[(end_y - 1) * 80 + x] = color;
        }
    }
    
    // Left and right borders
    for (int y = start_y; y < end_y && y < 25; y++) {
        if (start_x >= 0 && y >= 0 && start_x < 80) {
            screen_buffer[y * 80 + start_x] = '|';
            color_buffer[y * 80 + start_x] = color;
        }
        if (end_x - 1 >= 0 && y >= 0 && end_x - 1 < 80) {
            screen_buffer[y * 80 + (end_x - 1)] = '|';
            color_buffer[y * 80 + (end_x - 1)] = color;
        }
    }
}

void gui_draw_text(const char* text, int x, int y, color_t color) {
    if (!text) return;
    
    int char_x = x / 8;
    int char_y = y / 16;
    
    for (int i = 0; text[i] && char_x + i < 80 && char_y < 25; i++) {
        if (char_x + i >= 0 && char_y >= 0) {
            screen_buffer[char_y * 80 + char_x + i] = text[i];
            color_buffer[char_y * 80 + char_x + i] = color;
        }
    }
}

void gui_draw_window(window_t* window) {
    if (!window || !window->base.visible) return;
    
    // Draw window background
    gui_draw_rect(window->base.bounds, window->base.bg_color);
    
    // Draw window border
    if (window->has_border) {
        gui_draw_border(window->base.bounds, COLOR_BLACK);
    }
    
    // Draw title bar
    if (window->has_titlebar) {
        rect_t titlebar = {
            window->base.bounds.x,
            window->base.bounds.y,
            window->base.bounds.width,
            20
        };
        gui_draw_rect(titlebar, COLOR_BLUE);
        gui_draw_text(window->title, window->base.bounds.x + 8, window->base.bounds.y + 2, COLOR_WHITE);
    }
    
    // Draw child widgets
    widget_t* child = window->base.children;
    while (child) {
        gui_draw_widget(child);
        child = child->next_sibling;
    }
}

void gui_draw_widget(widget_t* widget) {
    if (!widget || !widget->visible) return;
    
    // Adjust coordinates relative to parent
    rect_t abs_bounds = widget->bounds;
    if (widget->parent) {
        abs_bounds.x += widget->parent->bounds.x;
        abs_bounds.y += widget->parent->bounds.y;
        if (widget->parent->type == WIDGET_WINDOW) {
            abs_bounds.y += 20; // Account for title bar
        }
    }
    
    switch (widget->type) {
        case WIDGET_BUTTON:
            gui_draw_rect(abs_bounds, widget->bg_color);
            gui_draw_border(abs_bounds, COLOR_BLACK);
            if (widget->text) {
                gui_draw_text(widget->text, abs_bounds.x + 4, abs_bounds.y + 4, widget->fg_color);
            }
            break;
            
        case WIDGET_LABEL:
            if (widget->text) {
                gui_draw_text(widget->text, abs_bounds.x, abs_bounds.y, widget->fg_color);
            }
            break;
            
        case WIDGET_TEXTBOX:
            gui_draw_rect(abs_bounds, widget->bg_color);
            gui_draw_border(abs_bounds, COLOR_BLACK);
            if (widget->text) {
                gui_draw_text(widget->text, abs_bounds.x + 2, abs_bounds.y + 2, widget->fg_color);
            }
            break;
            
        default:
            break;
    }
}

void gui_refresh_screen(void) {
    // Clear screen
    printf("\033[2J\033[H"); // ANSI clear screen and move cursor to home
    
    // Draw desktop
    gui_draw_rect(gui_mgr.desktop->bounds, gui_mgr.desktop_color);
    
    // Draw all windows
    window_t* window = gui_mgr.windows;
    while (window) {
        gui_draw_window(window);
        window = window->next;
    }
    
    // Output screen buffer (simplified text mode display)
    for (int y = 0; y < 25; y++) {
        for (int x = 0; x < 80; x++) {
            printf("%c", screen_buffer[y * 80 + x]);
        }
        printf("\n");
    }
    
    fflush(stdout);
}

int gui_poll_event(event_t* event) {
    // Simulate events for demonstration
    static int event_counter = 0;
    event_counter++;
    
    if (event_counter % 50 == 0) {
        event->type = EVENT_PAINT;
        return 1;
    }
    
    event->type = EVENT_NONE;
    return 0;
}

void gui_launch_app_handler(widget_t* widget, int x, int y) {
    (void)x; (void)y; // Suppress unused parameter warnings
    
    if (!widget || !widget->text) return;
    
    printf("GUI: Launching application: %s\n", widget->text);
    
    // Only launch applications if app_loader is available (main OS)
    // Standalone apps don't need this functionality
    #ifndef STANDALONE_APP
    // Launch the application using the app loader
    if (app_launch(widget->text) == 0) {
        printf("GUI: Successfully launched %s\n", widget->text);
    } else {
        printf("GUI: Failed to launch %s\n", widget->text);
    }
    #else
    printf("GUI: Standalone app - no launcher needed\n");
    #endif
}

void gui_handle_event(event_t* event) {
    if (!event) return;
    
    switch (event->type) {
        case EVENT_PAINT:
            // Refresh display
            break;
            
        case EVENT_MOUSE_PRESS:
            // Find widget at mouse position and dispatch click
            break;
            
        case EVENT_KEY_PRESS:
            // Handle keyboard input
            break;
            
        default:
            break;
    }
}
