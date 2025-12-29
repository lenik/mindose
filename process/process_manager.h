#ifndef PROCESS_MANAGER_H
#define PROCESS_MANAGER_H

#include <stdint.h>
#include <stddef.h>

// Executable format types
typedef enum {
    EXEC_FORMAT_UNKNOWN = 0,
    EXEC_FORMAT_ELF = 1,
    EXEC_FORMAT_PE = 2
} exec_format_t;

// ELF header structures (simplified)
typedef struct {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} elf_header_t;

// PE header structures (simplified)
typedef struct {
    uint16_t e_magic;
    uint16_t e_cblp;
    uint16_t e_cp;
    uint16_t e_crlc;
    uint16_t e_cparhdr;
    uint16_t e_minalloc;
    uint16_t e_maxalloc;
    uint16_t e_ss;
    uint16_t e_sp;
    uint16_t e_csum;
    uint16_t e_ip;
    uint16_t e_cs;
    uint16_t e_lfarlc;
    uint16_t e_ovno;
    uint16_t e_res[4];
    uint16_t e_oemid;
    uint16_t e_oeminfo;
    uint16_t e_res2[10];
    uint32_t e_lfanew;
} pe_dos_header_t;

// Loaded executable information
typedef struct {
    exec_format_t format;
    void* entry_point;
    void* base_address;
    size_t image_size;
    char filename[256];
} loaded_executable_t;

// Function declarations
exec_format_t detect_executable_format(const void* data, size_t size);
int load_elf_executable(const void* data, size_t size, loaded_executable_t* exec);
int load_pe_executable(const void* data, size_t size, loaded_executable_t* exec);
int execute_loaded_program(loaded_executable_t* exec);

// High-level interface
uint32_t load_and_execute(const char* filepath);
int unload_executable(uint32_t pid);

#endif // PROCESS_MANAGER_H
