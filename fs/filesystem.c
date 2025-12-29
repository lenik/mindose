#define _GNU_SOURCE
#include "filesystem.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static filesystem_t fs_state = {0};

// Helper function to split path into components
static char** split_path(const char* path, size_t* count) {
    if (!path || path[0] != '/') return NULL;
    
    char* path_copy = strdup(path + 1); // Skip leading '/'
    if (!path_copy) return NULL;
    
    // Count components
    *count = 0;
    char* temp = strdup(path_copy);
    char* token = strtok(temp, "/");
    while (token) {
        (*count)++;
        token = strtok(NULL, "/");
    }
    free(temp);
    
    if (*count == 0) {
        free(path_copy);
        return NULL;
    }
    
    // Allocate array
    char** components = malloc(sizeof(char*) * (*count));
    if (!components) {
        free(path_copy);
        return NULL;
    }
    
    // Fill array
    size_t i = 0;
    token = strtok(path_copy, "/");
    while (token && i < *count) {
        components[i] = strdup(token);
        i++;
        token = strtok(NULL, "/");
    }
    
    free(path_copy);
    return components;
}

static void free_path_components(char** components, size_t count) {
    if (!components) return;
    for (size_t i = 0; i < count; i++) {
        free(components[i]);
    }
    free(components);
}

int filesystem_init(mindose_config_t* config) {
    printf("FileSystem: Initializing...\n");
    
    // Create root directory
    fs_state.root = malloc(sizeof(directory_t));
    if (!fs_state.root) return -1;
    
    strcpy(fs_state.root->name, "/");
    fs_state.root->files = NULL;
    fs_state.root->file_count = 0;
    fs_state.root->capacity = 0;
    fs_state.root->subdirs = NULL;
    fs_state.root->subdir_count = 0;
    fs_state.root->subdir_capacity = 0;
    fs_state.root->parent = NULL;
    
    fs_state.current_dir = fs_state.root;
    fs_state.next_inode = 1;
    
    // Create standard directories
    if (fs_create_standard_dirs() != 0) {
        fprintf(stderr, "FileSystem: Failed to create standard directories\n");
        return -1;
    }
    
    printf("FileSystem: Initialized with standard directory structure\n");
    return 0;
}

void filesystem_cleanup(void) {
    // TODO: Implement proper cleanup of directory tree
    if (fs_state.root) {
        free(fs_state.root);
        fs_state.root = NULL;
    }
}

int fs_create_standard_dirs(void) {
    const char* standard_dirs[] = {
        "/bin",
        "/home",
        "/mnt",
        "/etc",
        "/dev"
    };
    
    size_t num_dirs = sizeof(standard_dirs) / sizeof(standard_dirs[0]);
    
    for (size_t i = 0; i < num_dirs; i++) {
        if (!fs_create_directory(standard_dirs[i])) {
            return -1;
        }
        printf("FileSystem: Created directory %s\n", standard_dirs[i]);
    }
    
    return 0;
}

directory_t* fs_create_directory(const char* path) {
    if (!path || path[0] != '/') return NULL;
    
    size_t count;
    char** components = split_path(path, &count);
    if (!components) return fs_state.root; // Root directory
    
    directory_t* current = fs_state.root;
    
    for (size_t i = 0; i < count; i++) {
        // Check if subdirectory already exists
        directory_t* found = NULL;
        for (size_t j = 0; j < current->subdir_count; j++) {
            if (strcmp(current->subdirs[j].name, components[i]) == 0) {
                found = &current->subdirs[j];
                break;
            }
        }
        
        if (found) {
            current = found;
        } else {
            // Create new subdirectory
            if (current->subdir_count >= current->subdir_capacity) {
                size_t new_capacity = current->subdir_capacity == 0 ? 4 : current->subdir_capacity * 2;
                directory_t* new_subdirs = realloc(current->subdirs, sizeof(directory_t) * new_capacity);
                if (!new_subdirs) {
                    free_path_components(components, count);
                    return NULL;
                }
                current->subdirs = new_subdirs;
                current->subdir_capacity = new_capacity;
            }
            
            directory_t* new_dir = &current->subdirs[current->subdir_count];
            strcpy(new_dir->name, components[i]);
            new_dir->files = NULL;
            new_dir->file_count = 0;
            new_dir->capacity = 0;
            new_dir->subdirs = NULL;
            new_dir->subdir_count = 0;
            new_dir->subdir_capacity = 0;
            new_dir->parent = current;
            
            current->subdir_count++;
            current = new_dir;
        }
    }
    
    free_path_components(components, count);
    return current;
}

directory_t* fs_find_directory(const char* path) {
    if (!path) return NULL;
    if (strcmp(path, "/") == 0) return fs_state.root;
    
    size_t count;
    char** components = split_path(path, &count);
    if (!components) return fs_state.root;
    
    directory_t* current = fs_state.root;
    
    for (size_t i = 0; i < count; i++) {
        directory_t* found = NULL;
        for (size_t j = 0; j < current->subdir_count; j++) {
            if (strcmp(current->subdirs[j].name, components[i]) == 0) {
                found = &current->subdirs[j];
                break;
            }
        }
        
        if (!found) {
            free_path_components(components, count);
            return NULL;
        }
        
        current = found;
    }
    
    free_path_components(components, count);
    return current;
}

int fs_change_directory(const char* path) {
    directory_t* dir = fs_find_directory(path);
    if (!dir) return -1;
    
    fs_state.current_dir = dir;
    return 0;
}

char* fs_get_current_path(void) {
    // Build path from current directory back to root
    char* path = malloc(1024);
    if (!path) return NULL;
    
    if (fs_state.current_dir == fs_state.root) {
        strcpy(path, "/");
        return path;
    }
    
    // TODO: Implement full path reconstruction
    strcpy(path, "/current");
    return path;
}

int fs_create_file(const char* path, const void* data, size_t size) {
    // Extract directory and filename
    char* path_copy = strdup(path);
    char* filename = strrchr(path_copy, '/');
    if (!filename) {
        free(path_copy);
        return -1;
    }
    
    *filename = '\0';
    filename++;
    
    directory_t* dir;
    if (strlen(path_copy) == 0) {
        dir = fs_state.root;
    } else {
        dir = fs_find_directory(path_copy);
        if (!dir) {
            free(path_copy);
            return -1;
        }
    }
    
    // Check if file already exists
    for (size_t i = 0; i < dir->file_count; i++) {
        if (strcmp(dir->files[i].name, filename) == 0) {
            free(path_copy);
            return -1; // File exists
        }
    }
    
    // Expand files array if needed
    if (dir->file_count >= dir->capacity) {
        size_t new_capacity = dir->capacity == 0 ? 4 : dir->capacity * 2;
        file_entry_t* new_files = realloc(dir->files, sizeof(file_entry_t) * new_capacity);
        if (!new_files) {
            free(path_copy);
            return -1;
        }
        dir->files = new_files;
        dir->capacity = new_capacity;
    }
    
    // Create file entry
    file_entry_t* file = &dir->files[dir->file_count];
    strcpy(file->name, filename);
    file->size = size;
    file->inode = fs_state.next_inode++;
    file->is_directory = 0;
    
    if (size > 0 && data) {
        file->data = malloc(size);
        if (!file->data) {
            free(path_copy);
            return -1;
        }
        memcpy(file->data, data, size);
    } else {
        file->data = NULL;
    }
    
    dir->file_count++;
    free(path_copy);
    return 0;
}

void fs_list_directory(const char* path) {
    directory_t* dir = path ? fs_find_directory(path) : fs_state.current_dir;
    if (!dir) {
        printf("Directory not found: %s\n", path);
        return;
    }
    
    printf("Directory listing for %s:\n", path ? path : "current");
    
    // List subdirectories
    for (size_t i = 0; i < dir->subdir_count; i++) {
        printf("  [DIR]  %s/\n", dir->subdirs[i].name);
    }
    
    // List files
    for (size_t i = 0; i < dir->file_count; i++) {
        printf("  [FILE] %s (%u bytes)\n", dir->files[i].name, dir->files[i].size);
    }
}
