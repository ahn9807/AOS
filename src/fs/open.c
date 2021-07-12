#include "vfs.h"
#include "spin_lock.h"
#include "stat.h"
#include "printf.h"
#include "debug.h"
#include "kmalloc.h"
#include "string.h"

int vfs_open(struct inode *inode, struct file *file) {
    if(!file) {
        return -FS_NOT_FILE;
    }

    file->inode = inode;
    file->f_op = inode->i_fop;
    file->offset = 0;
    file->inode->refcount++;

    return 0;
}

int vfs_open_by_path(char* path, struct file *file) {
    struct vfs_node *node = vfs_mountpoint(path);

    if(node == NULL || node->inode == NULL) {
        return -FS_NO_ENTRY;
    }

    inode_t *root_node = node->inode;
    int error_code = 0;
    struct dentry dir;
    // get relative path from mount point
    char *lookup_path = path + strlen(node->full_path);

    if((error_code = vfs_lookup(root_node, lookup_path, &dir)) != FS_FILE) {
        return -FS_NOT_FILE;
    }

    if(dir.inode != NULL && !S_ISDIR(dir.inode->type)) {
        vfs_open(dir.inode, file);
        file->name = path_get_name(path);
    } else {
        return -FS_NOT_FILE;
    }

    return 0;
}