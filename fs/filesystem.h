#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <stdint.h>
#include <stddef.h>
#include "../common.h"

// File system structures
typedef struct {
    char name[256];
    uint32_t size;
    uint32_t inode;
    int is_directory;
    void* data;
} file_entry_t;

typedef struct directory {
    char name[256];
    file_entry_t* files;
    size_t file_count;
    size_t capacity;
    struct directory* subdirs;
    size_t subdir_count;
    size_t subdir_capacity;
    struct directory* parent;
} directory_t;

typedef struct {
    directory_t* root;
    directory_t* current_dir;
    uint32_t next_inode;
} filesystem_t;

// File handle for open files
typedef struct {
    file_entry_t* file;
    size_t position;
    int mode; // 0=read, 1=write, 2=append
    int is_open;
} file_handle_t;

// Function declarations
int filesystem_init(mindose_config_t* config);
void filesystem_cleanup(void);

// Directory operations
directory_t* fs_create_directory(const char* path);
directory_t* fs_find_directory(const char* path);
int fs_change_directory(const char* path);
char* fs_get_current_path(void);

// File operations
file_handle_t* fs_open_file(const char* path, int mode);
int fs_close_file(file_handle_t* handle);
size_t fs_read_file(file_handle_t* handle, void* buffer, size_t size);
size_t fs_write_file(file_handle_t* handle, const void* data, size_t size);
int fs_delete_file(const char* path);
int fs_create_file(const char* path, const void* data, size_t size);

// Utility functions
int fs_file_exists(const char* path);
size_t fs_get_file_size(const char* path);
void fs_list_directory(const char* path);

// Standard directories setup
int fs_create_standard_dirs(void);

#endif // FILESYSTEM_H
