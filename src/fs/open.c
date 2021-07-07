#include "vfs.h"
#include "spin_lock.h"

int vfs_open(struct inode *inode, struct file *file) {
    if(!file) {
        return -FS_NOT_FILE;
    }

    spin_lock(&file->inode->lock);
    file->inode = inode;
    file->f_op = inode->i_fop;
    file->offset = 0;
    file->inode->refcount++;
    spin_unlock(&file->inode->lock);
}

int vfs_open_by_path(char* path, struct file *file) {
    struct vfs_node *node = vfs_mountpoint(path);
    if(node == NULL) {
        return -FS_NO_ENTRY;
    }

    inode_t *inode = node->inode;
    struct dentry dir;
    
    int last_return = 1;
    // while(last_return != 0) {
    //     vfs_read
    // }
}