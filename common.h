#ifndef COMMON_H
#define COMMON_H

#include <stdint.h>
#include <stddef.h>

// Configuration structure used throughout Mindose
typedef struct {
    char* mem_size;
    char* diskimage;
    char* iso;
    char* arch;
    int application_mode;
} mindose_config_t;

#endif // COMMON_H
