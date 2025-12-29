#ifndef KERNEL_H
#define KERNEL_H

#include <stdint.h>
#include <stddef.h>
#include "../common.h"

// Memory management
typedef struct {
    void* base_addr;
    size_t size;
    int is_allocated;
} memory_block_t;

typedef struct {
    memory_block_t* blocks;
    size_t block_count;
    size_t total_memory;
} memory_manager_t;

// Process management
typedef struct process {
    uint32_t pid;
    char name[256];
    void* memory_base;
    size_t memory_size;
    int state; // 0=ready, 1=running, 2=blocked, 3=terminated
    struct process* next;
} process_t;

typedef struct {
    process_t* current_process;
    process_t* process_list;
    uint32_t next_pid;
} scheduler_t;

// Device management
typedef struct {
    char* mem_size;
    char* diskimage_path;
    char* iso_path;
    int devices_initialized;
} device_manager_t;

// Kernel state
typedef struct {
    memory_manager_t memory_mgr;
    scheduler_t scheduler;
    device_manager_t device_mgr;
    int initialized;
} kernel_state_t;

// Function declarations
int kernel_init(mindose_config_t* config);
void kernel_cleanup(void);

// Memory management functions
int memory_init(size_t total_size);
void* memory_alloc(size_t size);
void memory_free(void* ptr);
void memory_cleanup(void);

// Process management functions
int scheduler_init(void);
uint32_t process_create(const char* name, void* entry_point, size_t memory_size);
void process_switch(void);
void process_terminate(uint32_t pid);
process_t* process_get_current(void);

// Device management functions
int device_init(mindose_config_t* config);
void device_cleanup(void);

// System calls
int syscall_handler(int call_num, void* args);

#endif // KERNEL_H
