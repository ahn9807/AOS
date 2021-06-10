#include "vfs.h"
#include "vga_text.h"
#include "debug.h"
#include "kmalloc.h"

// Mount filesystem at super_node from path
void vfs_mount(const char *path, struct inode* super_node) {
    if(!strcmp(path, PATH_SEPARATOR_STRING)) {
        kfree(super_node);
        super_node = vfs_root;
    } else {
        struct vfs_node *local_fs = vfs_mountpoint(path);
        local_fs->fs = vfs_find("ext2");
        local_fs->fs->fs_op->mount(local_fs->inode->device, super_node, local_fs->aux);
    }
}