#define _GNU_SOURCE
#include "resource.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static resource_section_t global_resources = {0};

int resource_init(void) {
    global_resources.count = 0;
    global_resources.entries = NULL;
    printf("Resource: System initialized\n");
    return 0;
}

void resource_cleanup(void) {
    for (uint32_t i = 0; i < global_resources.count; i++) {
        if (global_resources.entries[i].data) {
            free(global_resources.entries[i].data);
        }
    }
    if (global_resources.entries) {
        free(global_resources.entries);
        global_resources.entries = NULL;
    }
    global_resources.count = 0;
}

int resource_add(uint32_t id, resource_type_t type, const void* data, size_t size) {
    if (!data || size == 0) return -1;
    
    // Check if resource already exists
    for (uint32_t i = 0; i < global_resources.count; i++) {
        if (global_resources.entries[i].id == id && global_resources.entries[i].type == type) {
            // Replace existing resource
            free(global_resources.entries[i].data);
            global_resources.entries[i].data = malloc(size);
            if (!global_resources.entries[i].data) return -1;
            memcpy(global_resources.entries[i].data, data, size);
            global_resources.entries[i].size = size;
            return 0;
        }
    }
    
    // Add new resource
    resource_entry_t* new_entries = realloc(global_resources.entries, 
                                           sizeof(resource_entry_t) * (global_resources.count + 1));
    if (!new_entries) return -1;
    
    global_resources.entries = new_entries;
    resource_entry_t* entry = &global_resources.entries[global_resources.count];
    
    entry->id = id;
    entry->type = type;
    entry->size = size;
    entry->data = malloc(size);
    if (!entry->data) return -1;
    
    memcpy(entry->data, data, size);
    global_resources.count++;
    
    return 0;
}

void* resource_get(uint32_t id, resource_type_t type) {
    for (uint32_t i = 0; i < global_resources.count; i++) {
        if (global_resources.entries[i].id == id && global_resources.entries[i].type == type) {
            return global_resources.entries[i].data;
        }
    }
    return NULL;
}

icon_t* resource_get_icon(uint32_t id) {
    return (icon_t*)resource_get(id, RESOURCE_ICON);
}

int resource_load_from_section(const resource_section_t* section) {
    if (!section) return -1;
    
    for (uint32_t i = 0; i < section->count; i++) {
        const resource_entry_t* entry = &section->entries[i];
        if (resource_add(entry->id, entry->type, entry->data, entry->size) != 0) {
            return -1;
        }
    }
    
    return 0;
}

icon_t* create_icon_16x16(const uint8_t* pixel_data) {
    if (!pixel_data) return NULL;
    
    icon_t* icon = malloc(sizeof(icon_t));
    if (!icon) return NULL;
    
    icon->width = 16;
    icon->height = 16;
    icon->colors = 16; // 4-bit color
    icon->reserved = 0;
    icon->planes = 1;
    icon->bits_per_pixel = 4;
    icon->size = 16 * 16 / 2; // 4 bits per pixel
    
    icon->data = malloc(icon->size);
    if (!icon->data) {
        free(icon);
        return NULL;
    }
    
    memcpy(icon->data, pixel_data, icon->size);
    return icon;
}

void destroy_icon(icon_t* icon) {
    if (!icon) return;
    if (icon->data) {
        free(icon->data);
    }
    free(icon);
}
