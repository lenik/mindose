#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "../../gui/gui.h"
#include "../../resource/resource.h"
#include "../../kernel/kernel.h"

// Minesweeper icon data (16x16, 4-bit color)
static const uint8_t minesweeper_icon_data[] = {
    0x00, 0x00, 0x00, 0x00, 0x0F, 0xFF, 0xFF, 0xF0,
    0x0F, 0x44, 0x44, 0xF0, 0x0F, 0x4E, 0xE4, 0xF0,
    0x0F, 0x4E, 0xE4, 0xF0, 0x0F, 0x44, 0x44, 0xF0,
    0x0F, 0x44, 0x44, 0xF0, 0x0F, 0x4C, 0xC4, 0xF0,
    0x0F, 0x4C, 0xC4, 0xF0, 0x0F, 0x44, 0x44, 0xF0,
    0x0F, 0x44, 0x44, 0xF0, 0x0F, 0x4A, 0xA4, 0xF0,
    0x0F, 0x4A, 0xA4, 0xF0, 0x0F, 0x44, 0x44, 0xF0,
    0x0F, 0xFF, 0xFF, 0xF0, 0x00, 0x00, 0x00, 0x00
};

// Resource section for this application
static resource_entry_t minesweeper_resources[] = {
    {ICON_MINESWEEPER, RESOURCE_ICON, sizeof(minesweeper_icon_data), (void*)minesweeper_icon_data}
};

static resource_section_t minesweeper_resource_section = {
    .count = 1,
    .entries = minesweeper_resources
};

#define MINESWEEPER_ROWS 10
#define MINESWEEPER_COLS 10
#define MINESWEEPER_MINES 10

typedef struct {
    int is_mine;
    int is_revealed;
    int is_flagged;
    int adjacent_mines;
} minesweeper_cell_t;

// Application state
typedef struct {
    window_t* window;
    widget_t* cells[MINESWEEPER_ROWS][MINESWEEPER_COLS];
    widget_t* btn_new_game;
    widget_t* btn_exit;
    widget_t* lbl_status;
    minesweeper_cell_t grid[MINESWEEPER_ROWS][MINESWEEPER_COLS];
    int game_over;
    int mines_count;
    int flags_count;
    int revealed_count;
    int running;
} minesweeper_app_t;

static minesweeper_app_t app_state = {0};

void minesweeper_update_status(void) {
    if (!app_state.lbl_status) return;
    
    char status_text[50];
    if (app_state.game_over) {
        if (app_state.revealed_count == (MINESWEEPER_ROWS * MINESWEEPER_COLS - MINESWEEPER_MINES)) {
            strcpy(status_text, "YOU WIN!");
        } else {
            strcpy(status_text, "GAME OVER");
        }
    } else {
        snprintf(status_text, sizeof(status_text), "Mines: %d", app_state.mines_count - app_state.flags_count);
    }
    
    if (app_state.lbl_status->text) {
        free(app_state.lbl_status->text);
    }
    app_state.lbl_status->text = strdup(status_text);
}

void minesweeper_reveal_cell(int row, int col) {
    if (app_state.game_over) return;
    if (row < 0 || row >= MINESWEEPER_ROWS || col < 0 || col >= MINESWEEPER_COLS) return;
    if (app_state.grid[row][col].is_revealed || app_state.grid[row][col].is_flagged) return;
    
    app_state.grid[row][col].is_revealed = 1;
    app_state.revealed_count++;
    
    if (app_state.cells[row][col]) {
        app_state.cells[row][col]->bg_color = COLOR_WHITE;
        
        if (app_state.grid[row][col].is_mine) {
            // Hit a mine - game over
            if (app_state.cells[row][col]->text) free(app_state.cells[row][col]->text);
            app_state.cells[row][col]->text = strdup("*");
            app_state.cells[row][col]->bg_color = COLOR_RED;
            app_state.game_over = 1;
            
            // Reveal all mines
            for (int r = 0; r < MINESWEEPER_ROWS; r++) {
                for (int c = 0; c < MINESWEEPER_COLS; c++) {
                    if (app_state.grid[r][c].is_mine && app_state.cells[r][c]) {
                        if (app_state.cells[r][c]->text) free(app_state.cells[r][c]->text);
                        app_state.cells[r][c]->text = strdup("*");
                        app_state.cells[r][c]->bg_color = COLOR_RED;
                    }
                }
            }
            
            printf("Minesweeper: Game Over - Mine hit!\n");
        } else {
            // Safe cell
            if (app_state.grid[row][col].adjacent_mines > 0) {
                char count_str[2];
                snprintf(count_str, sizeof(count_str), "%d", app_state.grid[row][col].adjacent_mines);
                if (app_state.cells[row][col]->text) free(app_state.cells[row][col]->text);
                app_state.cells[row][col]->text = strdup(count_str);
            } else {
                // Empty cell - auto-reveal adjacent cells
                for (int dr = -1; dr <= 1; dr++) {
                    for (int dc = -1; dc <= 1; dc++) {
                        minesweeper_reveal_cell(row + dr, col + dc);
                    }
                }
            }
            
            // Check for win condition
            if (app_state.revealed_count == (MINESWEEPER_ROWS * MINESWEEPER_COLS - MINESWEEPER_MINES)) {
                app_state.game_over = 1;
                printf("Minesweeper: You Win!\n");
            }
        }
    }
    
    minesweeper_update_status();
}

void minesweeper_cell_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget) return;
    
    // Find which cell was clicked
    for (int row = 0; row < MINESWEEPER_ROWS; row++) {
        for (int col = 0; col < MINESWEEPER_COLS; col++) {
            if (app_state.cells[row][col] == widget) {
                minesweeper_reveal_cell(row, col);
                return;
            }
        }
    }
}

void minesweeper_new_game(void) {
    printf("Minesweeper: Starting new game\n");
    
    // Initialize grid
    for (int row = 0; row < MINESWEEPER_ROWS; row++) {
        for (int col = 0; col < MINESWEEPER_COLS; col++) {
            app_state.grid[row][col].is_mine = 0;
            app_state.grid[row][col].is_revealed = 0;
            app_state.grid[row][col].is_flagged = 0;
            app_state.grid[row][col].adjacent_mines = 0;
            
            // Reset cell appearance
            if (app_state.cells[row][col]) {
                if (app_state.cells[row][col]->text) {
                    free(app_state.cells[row][col]->text);
                }
                app_state.cells[row][col]->text = strdup("");
                app_state.cells[row][col]->bg_color = COLOR_LIGHT_GRAY;
            }
        }
    }
    
    // Place mines randomly
    srand(time(NULL));
    int mines_placed = 0;
    while (mines_placed < MINESWEEPER_MINES) {
        int row = rand() % MINESWEEPER_ROWS;
        int col = rand() % MINESWEEPER_COLS;
        
        if (!app_state.grid[row][col].is_mine) {
            app_state.grid[row][col].is_mine = 1;
            mines_placed++;
        }
    }
    
    // Calculate adjacent mine counts
    for (int row = 0; row < MINESWEEPER_ROWS; row++) {
        for (int col = 0; col < MINESWEEPER_COLS; col++) {
            if (!app_state.grid[row][col].is_mine) {
                int count = 0;
                for (int dr = -1; dr <= 1; dr++) {
                    for (int dc = -1; dc <= 1; dc++) {
                        int nr = row + dr;
                        int nc = col + dc;
                        if (nr >= 0 && nr < MINESWEEPER_ROWS && 
                            nc >= 0 && nc < MINESWEEPER_COLS &&
                            app_state.grid[nr][nc].is_mine) {
                            count++;
                        }
                    }
                }
                app_state.grid[row][col].adjacent_mines = count;
            }
        }
    }
    
    // Reset game state
    app_state.game_over = 0;
    app_state.flags_count = 0;
    app_state.revealed_count = 0;
    
    // Update status
    minesweeper_update_status();
}

void minesweeper_button_click(widget_t* widget, int x, int y) {
    (void)x; (void)y;
    
    if (!widget || !widget->text) return;
    
    if (strcmp(widget->text, "New Game") == 0) {
        minesweeper_new_game();
    } else if (strcmp(widget->text, "Exit") == 0) {
        app_state.running = 0;
    }
}

int minesweeper_init(void) {
    printf("Minesweeper App: Initializing standalone application...\n");
    
    // Load resources
    resource_init();
    resource_load_from_section(&minesweeper_resource_section);
    
    // Create main window
    app_state.window = gui_create_window("Minesweeper", 200, 150, 400, 350);
    if (!app_state.window) return -1;
    
    // Create control buttons
    app_state.btn_new_game = gui_create_button("New Game", 10, 10, 80, 25);
    app_state.btn_exit = gui_create_button("Exit", 100, 10, 60, 25);
    
    // Create status label
    app_state.lbl_status = gui_create_label("Mines: 10", 200, 15, 100, 20);
    
    // Set up button event handlers
    if (app_state.btn_new_game) app_state.btn_new_game->on_click = minesweeper_button_click;
    if (app_state.btn_exit) app_state.btn_exit->on_click = minesweeper_button_click;
    
    // Add control widgets to window
    if (app_state.btn_new_game) gui_add_child_widget(&app_state.window->base, app_state.btn_new_game);
    if (app_state.btn_exit) gui_add_child_widget(&app_state.window->base, app_state.btn_exit);
    if (app_state.lbl_status) gui_add_child_widget(&app_state.window->base, app_state.lbl_status);
    
    // Create game grid
    for (int row = 0; row < MINESWEEPER_ROWS; row++) {
        for (int col = 0; col < MINESWEEPER_COLS; col++) {
            int x = 10 + col * 25;
            int y = 50 + row * 25;
            
            app_state.cells[row][col] = gui_create_button("", x, y, 23, 23);
            if (app_state.cells[row][col]) {
                app_state.cells[row][col]->bg_color = COLOR_LIGHT_GRAY;
                app_state.cells[row][col]->on_click = minesweeper_cell_click;
                gui_add_child_widget(&app_state.window->base, app_state.cells[row][col]);
            }
        }
    }
    
    // Initialize game state
    app_state.game_over = 0;
    app_state.mines_count = MINESWEEPER_MINES;
    app_state.flags_count = 0;
    app_state.revealed_count = 0;
    app_state.running = 1;
    
    // Start new game
    minesweeper_new_game();
    
    gui_show_window(app_state.window);
    
    printf("Minesweeper App: Initialized successfully\n");
    return 0;
}

void minesweeper_cleanup(void) {
    if (app_state.window) {
        gui_destroy_window(app_state.window);
        app_state.window = NULL;
    }
    
    resource_cleanup();
    printf("Minesweeper App: Cleaned up\n");
}

void minesweeper_main_loop(void) {
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

// Main entry point for standalone Minesweeper application
int main(int argc, char* argv[]) {
    (void)argc; (void)argv;
    
    printf("Starting Minesweeper as standalone application...\n");
    
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
    
    if (minesweeper_init() != 0) {
        fprintf(stderr, "Failed to initialize Minesweeper\n");
        gui_cleanup();
        return 1;
    }
    
    // Run main loop
    minesweeper_main_loop();
    
    // Cleanup
    minesweeper_cleanup();
    gui_cleanup();
    
    printf("Minesweeper application terminated\n");
    return 0;
}
