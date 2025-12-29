#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include "../../gui/gui.h"
#include "../../resource/resource.h"
#include "../../kernel/kernel.h"

// Paint icon data (16x16, 4-bit color)
static const uint8_t paint_icon_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x04, 0x40,
    0x00, 0x00, 0x44, 0x40, 0x00, 0x04, 0x44, 0x00,
    0x00, 0x44, 0x40, 0x00, 0x04, 0x44, 0x00, 0x00,
    0x44, 0x40, 0x00, 0x00, 0x44, 0x00, 0x00, 0x00,
    0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

// Resource section for this application
static resource_entry_t paint_resources[] = {
    {ICON_PAINT, RESOURCE_ICON, sizeof(paint_icon_data), (void*)paint_icon_data}
};

static resource_section_t paint_resource_section = {
    .count = 1,
    .entries = paint_resources
};

typedef enum {
    PAINT_TOOL_BRUSH = 0,
    PAINT_TOOL_LINE,
    PAINT_TOOL_RECTANGLE,
    PAINT_TOOL_CIRCLE,
    PAINT_TOOL_ERASER
} paint_tool_t;

// Application state
typedef struct {
    window_t* window;
    widget_t* canvas;
    widget_t* btn_brush;
    widget_t* btn_line;
    widget_t* btn_rectangle;
    widget_t* btn_circle;
    widget_t* btn_clear;
    paint_tool_t current_tool;
    color_t current_color;
    int canvas_width;
    int canvas_height;
    int running;
} paint_app_t;

static paint_app_t app_state = {0};

void paint_tool_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget || !widget->text) return;
    
    if (strcmp(widget->text, "Brush") == 0) {
        app_state.current_tool = PAINT_TOOL_BRUSH;
    } else if (strcmp(widget->text, "Line") == 0) {
        app_state.current_tool = PAINT_TOOL_LINE;
    } else if (strcmp(widget->text, "Rectangle") == 0) {
        app_state.current_tool = PAINT_TOOL_RECTANGLE;
    } else if (strcmp(widget->text, "Circle") == 0) {
        app_state.current_tool = PAINT_TOOL_CIRCLE;
    } else if (strcmp(widget->text, "Clear") == 0) {
        if (app_state.canvas) {
            app_state.canvas->bg_color = COLOR_WHITE;
        }
        printf("Paint: Canvas cleared\n");
    }
    
    printf("Paint: Selected tool: %s\n", widget->text);
}

void paint_draw_pixel(int x, int y, color_t color) {
    printf("Paint: Drawing pixel at (%d, %d) with color %d\n", x, y, color);
}

void paint_draw_line(int x1, int y1, int x2, int y2, color_t color) {
    printf("Paint: Drawing line from (%d, %d) to (%d, %d)\n", x1, y1, x2, y2);
    paint_draw_pixel(x1, y1, color);
    paint_draw_pixel(x2, y2, color);
}

void paint_draw_rectangle(int x, int y, int width, int height, color_t color) {
    printf("Paint: Drawing rectangle at (%d, %d) size %dx%d\n", x, y, width, height);
    
    for (int i = 0; i < width; i++) {
        paint_draw_pixel(x + i, y, color);
        paint_draw_pixel(x + i, y + height - 1, color);
    }
    for (int i = 0; i < height; i++) {
        paint_draw_pixel(x, y + i, color);
        paint_draw_pixel(x + width - 1, y + i, color);
    }
}

void paint_draw_circle(int cx, int cy, int radius, color_t color) {
    printf("Paint: Drawing circle at (%d, %d) radius %d\n", cx, cy, radius);
    
    for (int angle = 0; angle < 360; angle += 10) {
        int x = cx + (int)(radius * cos(angle * 3.14159 / 180));
        int y = cy + (int)(radius * sin(angle * 3.14159 / 180));
        paint_draw_pixel(x, y, color);
    }
}

int paint_init(void) {
    printf("Paint App: Initializing standalone application...\n");
    
    // Load resources
    resource_init();
    resource_load_from_section(&paint_resource_section);
    
    // Create main window
    app_state.window = gui_create_window("Paint", 150, 80, 600, 500);
    if (!app_state.window) return -1;
    
    // Create tool buttons
    app_state.btn_brush = gui_create_button("Brush", 10, 10, 60, 25);
    app_state.btn_line = gui_create_button("Line", 80, 10, 60, 25);
    app_state.btn_rectangle = gui_create_button("Rectangle", 150, 10, 80, 25);
    app_state.btn_circle = gui_create_button("Circle", 240, 10, 60, 25);
    app_state.btn_clear = gui_create_button("Clear", 310, 10, 60, 25);
    
    // Create canvas area
    app_state.canvas = gui_create_widget(WIDGET_WINDOW, 10, 45, 570, 420);
    if (app_state.canvas) {
        app_state.canvas->bg_color = COLOR_WHITE;
    }
    
    // Set up event handlers
    if (app_state.btn_brush) app_state.btn_brush->on_click = paint_tool_click;
    if (app_state.btn_line) app_state.btn_line->on_click = paint_tool_click;
    if (app_state.btn_rectangle) app_state.btn_rectangle->on_click = paint_tool_click;
    if (app_state.btn_circle) app_state.btn_circle->on_click = paint_tool_click;
    if (app_state.btn_clear) app_state.btn_clear->on_click = paint_tool_click;
    
    // Add widgets to window
    if (app_state.btn_brush) gui_add_child_widget(&app_state.window->base, app_state.btn_brush);
    if (app_state.btn_line) gui_add_child_widget(&app_state.window->base, app_state.btn_line);
    if (app_state.btn_rectangle) gui_add_child_widget(&app_state.window->base, app_state.btn_rectangle);
    if (app_state.btn_circle) gui_add_child_widget(&app_state.window->base, app_state.btn_circle);
    if (app_state.btn_clear) gui_add_child_widget(&app_state.window->base, app_state.btn_clear);
    if (app_state.canvas) gui_add_child_widget(&app_state.window->base, app_state.canvas);
    
    // Initialize canvas
    app_state.canvas_width = 570;
    app_state.canvas_height = 420;
    app_state.current_tool = PAINT_TOOL_BRUSH;
    app_state.current_color = COLOR_BLACK;
    app_state.running = 1;
    
    gui_show_window(app_state.window);
    
    printf("Paint App: Initialized successfully\n");
    return 0;
}

void paint_cleanup(void) {
    if (app_state.window) {
        gui_destroy_window(app_state.window);
        app_state.window = NULL;
    }
    
    resource_cleanup();
    printf("Paint App: Cleaned up\n");
}

void paint_main_loop(void) {
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

// Main entry point for standalone Paint application
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    printf("Starting Paint as standalone application...\n");
    
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
    
    if (paint_init() != 0) {
        fprintf(stderr, "Failed to initialize Paint\n");
        gui_cleanup();
        return 1;
    }
    
    // Run main loop
    paint_main_loop();
    
    // Cleanup
    paint_cleanup();
    gui_cleanup();
    
    printf("Paint application terminated\n");
    return 0;
}
