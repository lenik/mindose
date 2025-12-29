#ifndef GUI_H
#define GUI_H

#include <stdint.h>
#include <stddef.h>
#include "../common.h"

// Color definitions (Windows 3.1 style)
typedef enum {
    COLOR_BLACK = 0,
    COLOR_DARK_BLUE = 1,
    COLOR_DARK_GREEN = 2,
    COLOR_DARK_CYAN = 3,
    COLOR_DARK_RED = 4,
    COLOR_DARK_MAGENTA = 5,
    COLOR_BROWN = 6,
    COLOR_LIGHT_GRAY = 7,
    COLOR_DARK_GRAY = 8,
    COLOR_BLUE = 9,
    COLOR_GREEN = 10,
    COLOR_CYAN = 11,
    COLOR_RED = 12,
    COLOR_MAGENTA = 13,
    COLOR_YELLOW = 14,
    COLOR_WHITE = 15
} color_t;

// Rectangle structure
typedef struct {
    int x, y;
    int width, height;
} rect_t;

// Event types
typedef enum {
    EVENT_NONE = 0,
    EVENT_KEY_PRESS,
    EVENT_KEY_RELEASE,
    EVENT_MOUSE_MOVE,
    EVENT_MOUSE_PRESS,
    EVENT_MOUSE_RELEASE,
    EVENT_WINDOW_CLOSE,
    EVENT_WINDOW_RESIZE,
    EVENT_PAINT
} event_type_t;

// Event structure
typedef struct {
    event_type_t type;
    union {
        struct {
            int key_code;
            int modifiers;
        } key;
        struct {
            int x, y;
            int button; // 1=left, 2=right, 3=middle
        } mouse;
        struct {
            int width, height;
        } resize;
    } data;
} event_t;

// Widget types
typedef enum {
    WIDGET_WINDOW = 0,
    WIDGET_BUTTON,
    WIDGET_LABEL,
    WIDGET_TEXTBOX,
    WIDGET_MENU,
    WIDGET_MENUBAR
} widget_type_t;

// Widget structure
typedef struct widget {
    widget_type_t type;
    rect_t bounds;
    char* text;
    color_t bg_color;
    color_t fg_color;
    int visible;
    int enabled;
    struct widget* parent;
    struct widget* children;
    struct widget* next_sibling;
    void (*on_click)(struct widget* widget, int x, int y);
    void (*on_paint)(struct widget* widget);
} widget_t;

// Window structure
typedef struct window {
    widget_t base;
    char title[256];
    int minimized;
    int maximized;
    int has_titlebar;
    int has_border;
    int resizable;
    struct window* next;
} window_t;

// Desktop/GUI manager
typedef struct {
    window_t* windows;
    window_t* active_window;
    widget_t* desktop;
    int screen_width;
    int screen_height;
    color_t desktop_color;
    int initialized;
} gui_manager_t;

// Function declarations
int gui_init(mindose_config_t* config);
void gui_cleanup(void);
void gui_main_loop(void);

// Window management
window_t* gui_create_window(const char* title, int x, int y, int width, int height);
void gui_destroy_window(window_t* window);
void gui_show_window(window_t* window);
void gui_hide_window(window_t* window);
void gui_set_active_window(window_t* window);
void gui_minimize_window(window_t* window);
void gui_maximize_window(window_t* window);
void gui_restore_window(window_t* window);

// Widget management
widget_t* gui_create_widget(widget_type_t type, int x, int y, int width, int height);
void gui_destroy_widget(widget_t* widget);
void gui_add_child_widget(widget_t* parent, widget_t* child);
widget_t* gui_create_button(const char* text, int x, int y, int width, int height);
widget_t* gui_create_label(const char* text, int x, int y, int width, int height);
widget_t* gui_create_textbox(int x, int y, int width, int height);

// Drawing functions
void gui_draw_rect(rect_t rect, color_t color);
void gui_draw_border(rect_t rect, color_t color);
void gui_draw_text(const char* text, int x, int y, color_t color);
void gui_draw_window(window_t* window);
void gui_draw_widget(widget_t* widget);
void gui_refresh_screen(void);

// Event handling
int gui_poll_event(event_t* event);
void gui_handle_event(event_t* event);
void gui_dispatch_event(widget_t* widget, event_t* event);

// Utility functions
widget_t* gui_find_widget_at(int x, int y);
int gui_point_in_rect(int x, int y, rect_t rect);

// Application launcher
void gui_launch_app_handler(widget_t* widget, int x, int y);

#endif // GUI_H
