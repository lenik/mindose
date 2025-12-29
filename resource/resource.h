#ifndef RESOURCE_H
#define RESOURCE_H

#include <stdint.h>
#include <stddef.h>

// Resource types
typedef enum {
    RESOURCE_ICON = 1,
    RESOURCE_BITMAP = 2,
    RESOURCE_STRING = 3,
    RESOURCE_MENU = 4,
    RESOURCE_DIALOG = 5
} resource_type_t;

// Icon structure (16x16 pixels, 4-bit color)
typedef struct {
    uint8_t width;
    uint8_t height;
    uint8_t colors;
    uint8_t reserved;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t size;
    uint8_t* data;
} icon_t;

// Resource entry
typedef struct {
    uint32_t id;
    resource_type_t type;
    uint32_t size;
    void* data;
} resource_entry_t;

// Resource section
typedef struct {
    uint32_t count;
    resource_entry_t* entries;
} resource_section_t;

// Function declarations
int resource_init(void);
void resource_cleanup(void);
int resource_add(uint32_t id, resource_type_t type, const void* data, size_t size);
void* resource_get(uint32_t id, resource_type_t type);
icon_t* resource_get_icon(uint32_t id);
int resource_load_from_section(const resource_section_t* section);

// Icon creation helpers
icon_t* create_icon_16x16(const uint8_t* pixel_data);
void destroy_icon(icon_t* icon);

// Resource IDs for built-in icons
#define ICON_NOTEPAD     1001
#define ICON_PAINT       1002
#define ICON_MINESWEEPER 1003
#define ICON_CALCULATOR  1004
#define ICON_FILEMANAGER 1005
#define ICON_CLOCK       1006
#define ICON_CONTROLPANEL 1007
#define ICON_TERMINAL    1008
#define ICON_FOLDER     103
#define ICON_FILE       104

#endif // RESOURCE_H
