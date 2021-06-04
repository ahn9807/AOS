#include "ext2.h"
#include "vfs.h"
#include "kmalloc.h"

static int mount_ext2(struct inode *device, struct inode **ret) {
    struct ext2_superblock_t *sb = kmalloc(sizeof(ext2_superblock_t));
    struct ext2_fs_t *fs = kmalloc(sizeof(ext2_fs_t));

}