#include "vfs.h"
#include "spin_lock.h"
#include "stat.h"
#include "printf.h"
#include "debug.h"
#include "kmalloc.h"
#include "string.h"
#include "vga_text.h"

int vfs_open(struct inode *inode, struct file *file)
{
    if (!file)
    {
        return -FS_NOT_FILE;
    }

    file->inode = inode;
    file->f_op = inode->i_fop;
    file->offset = 0;
    file->inode->refcount++;

    return 0;
}

int vfs_open_by_path(const char *path, struct file *file)
{
    struct vfs_node *node = vfs_mountpoint(path);

    if (node == NULL || node->inode == NULL)
    {
        return -FS_NO_ENTRY;
    }

    inode_t *root_node = node->inode;
    int error_code = -FS_NO_ENTRY;
    struct dentry dir;
    // get relative path from mount point
    char *new_path = strdup(path);
    char *lookup_path = new_path + strlen(node->full_path);

    if ((error_code = vfs_lookup(root_node, path, &dir)) != FS_FILE)
    {
        goto error;
    }

    if (dir.inode != NULL && !S_ISDIR(dir.inode->type))
    {
        vfs_open(dir.inode, file);
        file->name = path_get_name(path);
        goto done;
    }

error:
    kfree(new_path);
    return error_code;
done:
    kfree(new_path);
    return 0;
}