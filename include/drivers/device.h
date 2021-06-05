#pragma once

#include <stdint.h>
#include <stddef.h>
#include "list.h"

#define DEVICE_BLOCK 0
#define DEVICE_CHARACTER 1

#define DEVICE_ROOT "/dev/"
#define DEVICE_BLOCK_DEAFULT_NAME "disk"
#define DEVICE_CHAR_DEFAULT_NAME "char"

struct device_operations;
typedef struct device {
    char *name;
    char device_type;
    struct device_operations *dev_op;
    void *aux;
    struct list_elem elem;
} device_t;

struct device_operations {
    int (*init)(void);
    size_t (*read) (void *aux, size_t offset, size_t size, void *buf);
    size_t (*write)(void *aux, size_t offset, size_t size, void *buf);
    size_t (*block_size)(void *aux);
};

// init every devices and device metadata at the kernel entry.
// After this step, OS have to install newly connected devices manually.
void dev_init();
// Register devices
void dev_install(device_t *dev, char* path);
// Unregister devices
void dev_uninstall(device_t *dev);

size_t dev_read(device_t *dev, size_t offset, size_t size, void *buf);
size_t dev_write(device_t *dev, size_t offset, size_t size, void *buf);