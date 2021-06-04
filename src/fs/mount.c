#include "vfs.h"

void vfs_install_device(const char* device_name);

void vfs_mount(const char *path, struct inode* local_root) {
    if(!strcmp(path, PATH_SEPARATOR_STRING) == 0) {
        vfs_root = local_root;
    } else {
        vfs_bind(path, local_root);
    }
}