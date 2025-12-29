#include "process_manager.h"
#include "../kernel/kernel.h"
#include "../fs/filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ELF magic numbers
#define ELF_MAGIC_0 0x7f
#define ELF_MAGIC_1 'E'
#define ELF_MAGIC_2 'L'
#define ELF_MAGIC_3 'F'

// PE magic numbers
#define PE_DOS_MAGIC 0x5A4D  // "MZ"
#define PE_NT_MAGIC  0x00004550  // "PE\0\0"

exec_format_t detect_executable_format(const void* data, size_t size) {
    if (!data || size < 64) return EXEC_FORMAT_UNKNOWN;
    
    const uint8_t* bytes = (const uint8_t*)data;
    
    // Check for ELF format
    if (size >= sizeof(elf_header_t) &&
        bytes[0] == ELF_MAGIC_0 &&
        bytes[1] == ELF_MAGIC_1 &&
        bytes[2] == ELF_MAGIC_2 &&
        bytes[3] == ELF_MAGIC_3) {
        return EXEC_FORMAT_ELF;
    }
    
    // Check for PE format
    if (size >= sizeof(pe_dos_header_t)) {
        const pe_dos_header_t* dos_header = (const pe_dos_header_t*)data;
        if (dos_header->e_magic == PE_DOS_MAGIC) {
            // Additional PE validation could be done here
            return EXEC_FORMAT_PE;
        }
    }
    
    return EXEC_FORMAT_UNKNOWN;
}

int load_elf_executable(const void* data, size_t size, loaded_executable_t* exec) {
    if (!data || !exec || size < sizeof(elf_header_t)) return -1;
    
    const elf_header_t* elf_header = (const elf_header_t*)data;
    
    // Basic ELF validation
    if (elf_header->e_ident[0] != ELF_MAGIC_0 ||
        elf_header->e_ident[1] != ELF_MAGIC_1 ||
        elf_header->e_ident[2] != ELF_MAGIC_2 ||
        elf_header->e_ident[3] != ELF_MAGIC_3) {
        return -1;
    }
    
    // Allocate memory for the executable
    size_t image_size = size; // Simplified - should parse program headers
    void* base_addr = memory_alloc(image_size);
    if (!base_addr) {
        return -1;
    }
    
    // Copy executable data to allocated memory
    memcpy(base_addr, data, size);
    
    // Fill executable info
    exec->format = EXEC_FORMAT_ELF;
    exec->entry_point = (void*)(uintptr_t)elf_header->e_entry;
    exec->base_address = base_addr;
    exec->image_size = image_size;
    
    printf("ProcessManager: Loaded ELF executable (entry: 0x%x, size: %zu)\n", 
           elf_header->e_entry, image_size);
    
    return 0;
}

int load_pe_executable(const void* data, size_t size, loaded_executable_t* exec) {
    if (!data || !exec || size < sizeof(pe_dos_header_t)) return -1;
    
    const pe_dos_header_t* dos_header = (const pe_dos_header_t*)data;
    
    // Basic PE validation
    if (dos_header->e_magic != PE_DOS_MAGIC) {
        return -1;
    }
    
    // Allocate memory for the executable
    size_t image_size = size; // Simplified - should parse PE headers properly
    void* base_addr = memory_alloc(image_size);
    if (!base_addr) {
        return -1;
    }
    
    // Copy executable data to allocated memory
    memcpy(base_addr, data, size);
    
    // Fill executable info (simplified entry point calculation)
    exec->format = EXEC_FORMAT_PE;
    exec->entry_point = base_addr; // Simplified
    exec->base_address = base_addr;
    exec->image_size = image_size;
    
    printf("ProcessManager: Loaded PE executable (size: %zu)\n", image_size);
    
    return 0;
}

int execute_loaded_program(loaded_executable_t* exec) {
    if (!exec || !exec->entry_point) return -1;
    
    // Create a new process for the executable
    uint32_t pid = process_create(exec->filename, exec->entry_point, exec->image_size);
    if (pid == 0) {
        printf("ProcessManager: Failed to create process for %s\n", exec->filename);
        return -1;
    }
    
    printf("ProcessManager: Started process %s (PID: %u)\n", exec->filename, pid);
    
    // In a real implementation, we would:
    // 1. Set up the process context
    // 2. Initialize registers and stack
    // 3. Jump to the entry point
    // For now, we just simulate execution
    
    return pid;
}

uint32_t load_and_execute(const char* filepath) {
    if (!filepath) return 0;
    
    printf("ProcessManager: Loading executable: %s\n", filepath);
    
    // For now, simulate loading from file system
    // In a real implementation, we would read the file from the file system
    
    // Create a dummy executable for demonstration
    const char dummy_elf[] = {
        0x7f, 'E', 'L', 'F',  // ELF magic
        0x01, 0x01, 0x01, 0x00,  // Class, data, version, OS/ABI
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // Padding
        0x02, 0x00,  // Type: executable
        0x3e, 0x00,  // Machine: x86-64
        0x01, 0x00, 0x00, 0x00,  // Version
        0x00, 0x10, 0x40, 0x00,  // Entry point (0x401000)
        // ... rest would be filled in real implementation
    };
    
    loaded_executable_t exec = {0};
    strncpy(exec.filename, filepath, sizeof(exec.filename) - 1);
    
    exec_format_t format = detect_executable_format(dummy_elf, sizeof(dummy_elf));
    
    int load_result = -1;
    switch (format) {
        case EXEC_FORMAT_ELF:
            load_result = load_elf_executable(dummy_elf, sizeof(dummy_elf), &exec);
            break;
        case EXEC_FORMAT_PE:
            load_result = load_pe_executable(dummy_elf, sizeof(dummy_elf), &exec);
            break;
        default:
            printf("ProcessManager: Unknown executable format for %s\n", filepath);
            return 0;
    }
    
    if (load_result != 0) {
        printf("ProcessManager: Failed to load executable %s\n", filepath);
        return 0;
    }
    
    return execute_loaded_program(&exec);
}

int unload_executable(uint32_t pid) {
    process_terminate(pid);
    return 0;
}
