#define _GNU_SOURCE
#include "kernel.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static kernel_state_t kernel_state = {0};

// Parse memory size string (e.g., "512M", "1G") to bytes
static size_t parse_memory_size(const char* size_str) {
    if (!size_str) return 256 * 1024 * 1024; // Default 256MB
    
    size_t size = 0;
    char unit = 0;
    
    if (sscanf(size_str, "%zu%c", &size, &unit) >= 1) {
        switch (unit) {
            case 'K': case 'k': return size * 1024;
            case 'M': case 'm': return size * 1024 * 1024;
            case 'G': case 'g': return size * 1024 * 1024 * 1024;
            default: return size;
        }
    }
    
    return 256 * 1024 * 1024; // Default fallback
}

int kernel_init(mindose_config_t* config) {
    printf("Kernel: Initializing core systems...\n");
    
    // Initialize memory management
    size_t mem_size = parse_memory_size(config->mem_size);
    if (memory_init(mem_size) != 0) {
        fprintf(stderr, "Kernel: Failed to initialize memory management\n");
        return -1;
    }
    
    // Initialize scheduler
    if (scheduler_init() != 0) {
        fprintf(stderr, "Kernel: Failed to initialize scheduler\n");
        return -1;
    }
    
    // Initialize device management
    if (device_init(config) != 0) {
        fprintf(stderr, "Kernel: Failed to initialize device management\n");
        return -1;
    }
    
    kernel_state.initialized = 1;
    printf("Kernel: Initialization complete\n");
    return 0;
}

void kernel_cleanup(void) {
    if (!kernel_state.initialized) return;
    
    printf("Kernel: Shutting down...\n");
    device_cleanup();
    memory_cleanup();
    kernel_state.initialized = 0;
}

// Memory Management Implementation
int memory_init(size_t total_size) {
    kernel_state.memory_mgr.total_memory = total_size;
    kernel_state.memory_mgr.block_count = 1024; // Max blocks
    
    kernel_state.memory_mgr.blocks = malloc(sizeof(memory_block_t) * kernel_state.memory_mgr.block_count);
    if (!kernel_state.memory_mgr.blocks) {
        return -1;
    }
    
    // Initialize first block as free memory
    kernel_state.memory_mgr.blocks[0].base_addr = malloc(total_size);
    kernel_state.memory_mgr.blocks[0].size = total_size;
    kernel_state.memory_mgr.blocks[0].is_allocated = 0;
    
    if (!kernel_state.memory_mgr.blocks[0].base_addr) {
        free(kernel_state.memory_mgr.blocks);
        return -1;
    }
    
    printf("Memory: Initialized %zu bytes\n", total_size);
    return 0;
}

void* memory_alloc(size_t size) {
    // Simple first-fit allocation
    for (size_t i = 0; i < kernel_state.memory_mgr.block_count; i++) {
        memory_block_t* block = &kernel_state.memory_mgr.blocks[i];
        
        if (!block->is_allocated && block->size >= size && block->base_addr) {
            if (block->size > size) {
                // Split block if it's larger than needed
                for (size_t j = 0; j < kernel_state.memory_mgr.block_count; j++) {
                    if (!kernel_state.memory_mgr.blocks[j].base_addr) {
                        kernel_state.memory_mgr.blocks[j].base_addr = (char*)block->base_addr + size;
                        kernel_state.memory_mgr.blocks[j].size = block->size - size;
                        kernel_state.memory_mgr.blocks[j].is_allocated = 0;
                        break;
                    }
                }
                block->size = size;
            }
            
            block->is_allocated = 1;
            return block->base_addr;
        }
    }
    
    return NULL; // No suitable block found
}

void memory_free(void* ptr) {
    for (size_t i = 0; i < kernel_state.memory_mgr.block_count; i++) {
        if (kernel_state.memory_mgr.blocks[i].base_addr == ptr) {
            kernel_state.memory_mgr.blocks[i].is_allocated = 0;
            // TODO: Implement block coalescing
            return;
        }
    }
}

void memory_cleanup(void) {
    if (kernel_state.memory_mgr.blocks) {
        if (kernel_state.memory_mgr.blocks[0].base_addr) {
            free(kernel_state.memory_mgr.blocks[0].base_addr);
        }
        free(kernel_state.memory_mgr.blocks);
        kernel_state.memory_mgr.blocks = NULL;
    }
}

// Scheduler Implementation
int scheduler_init(void) {
    kernel_state.scheduler.current_process = NULL;
    kernel_state.scheduler.process_list = NULL;
    kernel_state.scheduler.next_pid = 1;
    
    printf("Scheduler: Initialized\n");
    return 0;
}

uint32_t process_create(const char* name, void* entry_point, size_t memory_size) {
    process_t* process = malloc(sizeof(process_t));
    if (!process) return 0;
    
    process->pid = kernel_state.scheduler.next_pid++;
    strncpy(process->name, name, sizeof(process->name) - 1);
    process->name[sizeof(process->name) - 1] = '\0';
    
    process->memory_base = memory_alloc(memory_size);
    if (!process->memory_base) {
        free(process);
        return 0;
    }
    
    process->memory_size = memory_size;
    process->state = 0; // Ready
    
    // Add to process list
    process->next = kernel_state.scheduler.process_list;
    kernel_state.scheduler.process_list = process;
    
    printf("Process: Created '%s' (PID: %u)\n", name, process->pid);
    return process->pid;
}

void process_switch(void) {
    // Simple round-robin scheduling
    if (kernel_state.scheduler.current_process && kernel_state.scheduler.current_process->next) {
        kernel_state.scheduler.current_process = kernel_state.scheduler.current_process->next;
    } else {
        kernel_state.scheduler.current_process = kernel_state.scheduler.process_list;
    }
}

void process_terminate(uint32_t pid) {
    process_t* prev = NULL;
    process_t* current = kernel_state.scheduler.process_list;
    
    while (current) {
        if (current->pid == pid) {
            if (prev) {
                prev->next = current->next;
            } else {
                kernel_state.scheduler.process_list = current->next;
            }
            
            if (kernel_state.scheduler.current_process == current) {
                kernel_state.scheduler.current_process = current->next;
            }
            
            memory_free(current->memory_base);
            free(current);
            printf("Process: Terminated PID %u\n", pid);
            return;
        }
        prev = current;
        current = current->next;
    }
}

process_t* process_get_current(void) {
    return kernel_state.scheduler.current_process;
}

// Device Management Implementation
int device_init(mindose_config_t* config) {
    kernel_state.device_mgr.mem_size = config->mem_size ? strdup(config->mem_size) : NULL;
    kernel_state.device_mgr.diskimage_path = config->diskimage ? strdup(config->diskimage) : NULL;
    kernel_state.device_mgr.iso_path = config->iso ? strdup(config->iso) : NULL;
    
    printf("Devices: Initializing virtual devices...\n");
    
    if (kernel_state.device_mgr.diskimage_path) {
        printf("Devices: Disk image: %s\n", kernel_state.device_mgr.diskimage_path);
    }
    
    if (kernel_state.device_mgr.iso_path) {
        printf("Devices: ISO image: %s\n", kernel_state.device_mgr.iso_path);
    }
    
    kernel_state.device_mgr.devices_initialized = 1;
    return 0;
}

void device_cleanup(void) {
    if (kernel_state.device_mgr.mem_size) {
        free(kernel_state.device_mgr.mem_size);
        kernel_state.device_mgr.mem_size = NULL;
    }
    
    if (kernel_state.device_mgr.diskimage_path) {
        free(kernel_state.device_mgr.diskimage_path);
        kernel_state.device_mgr.diskimage_path = NULL;
    }
    
    if (kernel_state.device_mgr.iso_path) {
        free(kernel_state.device_mgr.iso_path);
        kernel_state.device_mgr.iso_path = NULL;
    }
    
    kernel_state.device_mgr.devices_initialized = 0;
}

// System Call Handler
int syscall_handler(int call_num, void* args) {
    switch (call_num) {
        case 1: // Memory allocation
            return (int)(intptr_t)memory_alloc(*(size_t*)args);
        case 2: // Memory free
            memory_free(args);
            return 0;
        case 3: // Process create
            // TODO: Implement process creation syscall
            return 0;
        default:
            return -1; // Unknown system call
    }
}
