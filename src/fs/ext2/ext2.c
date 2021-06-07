#include "ext2.h"
#include "vfs.h"
#include "kmalloc.h"

static int mount_ext2(struct inode *device, struct inode **ret) {
    struct ext2_superblock_t *sb = kmalloc(sizeof(ext2_superblock_t));
    struct ext2_fs_t *fs = kmalloc(sizeof(ext2_fs_t));

}

// Get Inode offset
static inline size_t get_ext2_inode_offset(ext2_fs_t *ext2, inode_number_t ino) {
    if(ino > ext2->superblock->inodes_count) {
        return -FS_INVALID;
    }

    uint32_t group_id = (ino - 1) / ext2->superblock->blocks_per_group;
    uint32_t inode_id = (ino - 1) % ext2->superblock->blocks_per_group;

    if(group_id > ext2->desc_size) {
        return -FS_INVALID;
    }

    ext2_group_desc_t *desc = &ext2->group_descs[group_id];
    // return desc->inode_table * 
}

// Read ext2_inode from disk with desc and ino
static int read_vfs_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t **ext2_inode_ref) {

}

// Find ext2_inode from disk and make vfs_inode_ref
static int make_vfs_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t **vfs_inode_ref) {

    // read_ext2_inode(desc, )
}