#include "vfs.h"
#include "spin_lock.h"

int vfs_open(struct inode *inode, struct file *file) {
    if(!file) {
        return -FS_NOT_FILE;
    }

    spin_lock(&file->inode->lock);
    file->inode->refcount++;
    spin_unlock(&file->inode->lock);
}