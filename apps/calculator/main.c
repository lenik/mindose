#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "../../gui/gui.h"
#include "../../resource/resource.h"
#include "../../kernel/kernel.h"

// Calculator icon data (16x16, 4-bit color)
static const uint8_t calculator_icon_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x88, 0x88, 0xF0,
    0x0F, 0x00, 0x00, 0xF0, 0x0F, 0x77, 0x77, 0xF0,
    0x0F, 0x77, 0x77, 0xF0, 0x0F, 0x77, 0x77, 0xF0,
    0x0F, 0x77, 0x77, 0xF0, 0x0F, 0x77, 0x77, 0xF0,
    0x0F, 0x77, 0x77, 0xF0, 0x0F, 0x77, 0x77, 0xF0,
    0x0F, 0x77, 0x77, 0xF0, 0x0F, 0xFF, 0xFF, 0xF0,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Resource section for this application
static resource_entry_t calculator_resources[] = {
    {ICON_CALCULATOR, RESOURCE_ICON, sizeof(calculator_icon_data), (void*)calculator_icon_data}
};

static resource_section_t calculator_resource_section = {
    .count = 1,
    .entries = calculator_resources
};

// Calculator state
typedef struct {
    window_t* window;
    widget_t* display;
    widget_t* buttons[20]; // 0-9, +, -, *, /, =, C, CE, ., +/-, sqrt
    double current_value;
    double stored_value;
    char operation;
    int new_number;
    char display_text[32];
    int running;
} calculator_app_t;

static calculator_app_t app_state = {0};

void calculator_update_display(void) {
    if (!app_state.display) return;
    
    if (app_state.display->text) {
        free(app_state.display->text);
    }
    app_state.display->text = strdup(app_state.display_text);
}

void calculator_clear(void) {
    app_state.current_value = 0.0;
    app_state.stored_value = 0.0;
    app_state.operation = 0;
    app_state.new_number = 1;
    strcpy(app_state.display_text, "0");
    calculator_update_display();
}

void calculator_perform_operation(void) {
    if (app_state.operation == 0) return;
    
    double result = app_state.stored_value;
    
    switch (app_state.operation) {
        case '+':
            result = app_state.stored_value + app_state.current_value;
            break;
        case '-':
            result = app_state.stored_value - app_state.current_value;
            break;
        case '*':
            result = app_state.stored_value * app_state.current_value;
            break;
        case '/':
            if (app_state.current_value != 0.0) {
                result = app_state.stored_value / app_state.current_value;
            } else {
                strcpy(app_state.display_text, "Error");
                calculator_update_display();
                return;
            }
            break;
    }
    
    app_state.current_value = result;
    app_state.stored_value = result;
    
    // Format the result
    if (result == (int)result) {
        snprintf(app_state.display_text, sizeof(app_state.display_text), "%.0f", result);
    } else {
        snprintf(app_state.display_text, sizeof(app_state.display_text), "%.6g", result);
    }
    
    calculator_update_display();
    app_state.new_number = 1;
}

void calculator_button_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget || !widget->text) return;
    
    const char* text = widget->text;
    
    // Number buttons
    if (text[0] >= '0' && text[0] <= '9' && text[1] == '\0') {
        if (app_state.new_number) {
            strcpy(app_state.display_text, text);
            app_state.current_value = atof(text);
            app_state.new_number = 0;
        } else {
            if (strlen(app_state.display_text) < 15) {
                strcat(app_state.display_text, text);
                app_state.current_value = atof(app_state.display_text);
            }
        }
        calculator_update_display();
        return;
    }
    
    // Decimal point
    if (strcmp(text, ".") == 0) {
        if (app_state.new_number) {
            strcpy(app_state.display_text, "0.");
            app_state.new_number = 0;
        } else if (!strchr(app_state.display_text, '.')) {
            strcat(app_state.display_text, ".");
        }
        calculator_update_display();
        return;
    }
    
    // Clear
    if (strcmp(text, "C") == 0) {
        calculator_clear();
        return;
    }
    
    // Clear Entry
    if (strcmp(text, "CE") == 0) {
        strcpy(app_state.display_text, "0");
        app_state.current_value = 0.0;
        app_state.new_number = 1;
        calculator_update_display();
        return;
    }
    
    // Plus/Minus
    if (strcmp(text, "+/-") == 0) {
        app_state.current_value = -app_state.current_value;
        if (app_state.current_value == (int)app_state.current_value) {
            snprintf(app_state.display_text, sizeof(app_state.display_text), "%.0f", app_state.current_value);
        } else {
            snprintf(app_state.display_text, sizeof(app_state.display_text), "%.6g", app_state.current_value);
        }
        calculator_update_display();
        return;
    }
    
    // Square root
    if (strcmp(text, "√") == 0) {
        if (app_state.current_value >= 0) {
            app_state.current_value = sqrt(app_state.current_value);
            if (app_state.current_value == (int)app_state.current_value) {
                snprintf(app_state.display_text, sizeof(app_state.display_text), "%.0f", app_state.current_value);
            } else {
                snprintf(app_state.display_text, sizeof(app_state.display_text), "%.6g", app_state.current_value);
            }
        } else {
            strcpy(app_state.display_text, "Error");
        }
        calculator_update_display();
        app_state.new_number = 1;
        return;
    }
    
    // Equals
    if (strcmp(text, "=") == 0) {
        calculator_perform_operation();
        app_state.operation = 0;
        return;
    }
    
    // Operations
    if (text[0] == '+' || text[0] == '-' || text[0] == '*' || text[0] == '/') {
        if (app_state.operation != 0 && !app_state.new_number) {
            calculator_perform_operation();
        }
        app_state.operation = text[0];
        app_state.stored_value = app_state.current_value;
        app_state.new_number = 1;
        return;
    }
}

int calculator_init(void) {
    printf("Calculator App: Initializing standalone application...\n");
    
    // Load resources
    resource_init();
    resource_load_from_section(&calculator_resource_section);
    
    // Create main window
    app_state.window = gui_create_window("Calculator", 100, 100, 220, 280);
    if (!app_state.window) return -1;
    
    // Create display
    strcpy(app_state.display_text, "0");
    app_state.display = gui_create_label(app_state.display_text, 10, 10, 190, 30);
    if (app_state.display) {
        app_state.display->bg_color = COLOR_WHITE;
        gui_add_child_widget(&app_state.window->base, app_state.display);
    }
    
    // Button layout: 4x5 grid
    const char* button_labels[] = {
        "CE", "C", "+/-", "√",
        "7", "8", "9", "/",
        "4", "5", "6", "*",
        "1", "2", "3", "-",
        "0", ".", "=", "+"
    };
    
    int button_index = 0;
    for (int row = 0; row < 5; row++) {
        for (int col = 0; col < 4; col++) {
            int x = 10 + col * 48;
            int y = 50 + row * 40;
            int width = (col == 0 && row == 4) ? 96 : 45; // Make "0" button wider
            
            app_state.buttons[button_index] = gui_create_button(button_labels[button_index], x, y, width, 35);
            if (app_state.buttons[button_index]) {
                app_state.buttons[button_index]->on_click = calculator_button_click;
                gui_add_child_widget(&app_state.window->base, app_state.buttons[button_index]);
            }
            
            button_index++;
            if (col == 0 && row == 4) col++; // Skip next position for wide "0" button
        }
    }
    
    // Initialize calculator state
    calculator_clear();
    app_state.running = 1;
    
    gui_show_window(app_state.window);
    
    printf("Calculator App: Initialized successfully\n");
    return 0;
}

void calculator_cleanup(void) {
    if (app_state.window) {
        gui_destroy_window(app_state.window);
        app_state.window = NULL;
    }
    
    resource_cleanup();
    printf("Calculator App: Cleaned up\n");
}

void calculator_main_loop(void) {
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

// Main entry point for standalone Calculator application
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    printf("Starting Calculator as standalone application...\n");
    
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
    
    if (calculator_init() != 0) {
        fprintf(stderr, "Failed to initialize Calculator\n");
        gui_cleanup();
        return 1;
    }
    
    // Run main loop
    calculator_main_loop();
    
    // Cleanup
    calculator_cleanup();
    gui_cleanup();
    
    printf("Calculator application terminated\n");
    return 0;
}
