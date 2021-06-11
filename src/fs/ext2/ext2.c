#include "ext2.h"
#include "vfs.h"
#include "kmalloc.h"
#include "vga_text.h"
#include "debug.h"
#include "stat.h"

static int mount(device_t *device, struct inode *ret, void *aux);
static int build_vfs_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t *ret);
static inline size_t inode_offset(ext2_fs_t *ext2, inode_number_t ino);
static size_t read_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref);
static size_t write_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t *ext2_inode_ref);
static int ext2_mount(device_t *device, inode_t *super_node, void *aux);

struct fs_operations vfs_ext2_fs_operations = {
    .mount = &ext2_mount,
};

struct file_operations vfs_ext2_file_operations = {

};

struct inode_operations vfs_ext2_inode_operations = {

};

struct vfs_fs vfs_ext2 = {
    .name = "ext2",
    .fs_op = &vfs_ext2_fs_operations,
};

int ext2_init() {
    vfs_install(&vfs_ext2);
}

static int ext2_mount(device_t *device, inode_t *super_node, void *aux) {
    mount(device, super_node, aux);
}

static size_t ext2_read(inode_t *inode, size_t offset, size_t len, void *buf) {

}

static size_t ext2_write(inode_t *inode, size_t offset, size_t len, void *buf) {

}

static size_t ext2_readdir(inode_t *inode, size_t offset, dentry_t *dir) {
    if(S_ISDIR(inode->flags)) {
        return -FS_NOT_DIRECTORY;
    }

    ext2_inode_t *ext2_inode = kmalloc(sizeof(ext2_inode_t));
    read_inode(inode->file_system, inode->inode_num, ext2_inode);
}

static size_t ext2_mkdir(inode_t *inode, dentry_t *dir) {

}

static size_t ext2_truncate(inode_t *inode, size_t len) {

}

static int mount(device_t *device, struct inode *super_node, void *aux) {
    ext2_superblock_t *sb = kmalloc(sizeof(ext2_superblock_t));
    ext2_fs_t *fs = kmalloc(sizeof(ext2_fs_t));

    dev_read(device, 1024, sizeof(ext2_superblock_t), sb);
    printf("sb magic: 0x%x\n", sb->magic);

    if(sb->magic != EXT2_MAGIC) {
        return -FS_INVALID;
    }

    fs->superblock = sb;
    fs->block_size = 1024UL << sb->log_block_size;
    fs->desc_len = (sb->blocks_count + sb->blocks_per_group - 1)/ sb->blocks_per_group;
    fs->group_descs = kcalloc(fs->desc_len, sizeof(ext2_group_desc_t));
    fs->mountpoint = super_node;
    fs->mountpoint->device = device;
    aux = fs;

    dev_read(device, fs->block_size == 1024 ? 2048 : fs->block_size, fs->desc_len * sizeof(ext2_group_desc_t), fs->group_descs);
    build_vfs_inode(fs, 2, super_node);
}

static int build_vfs_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t *inode) {
    // read inode from disk / cache
    ext2_inode_t *inode_buf = kcalloc(1, sizeof(ext2_inode_t));
    read_inode(ext2, 1, inode_buf);

    inode->inode_num = ino;
    inode->size = inode_buf->size;
    inode->mode = inode_buf->mode;
    inode->uid = inode_buf->uid;
    inode->gid = inode_buf->gid;
    inode->nlink = inode_buf->links_count;

    inode->atime = inode_buf->atime;
    inode->ctime = inode_buf->ctime;
    inode->mtime = inode_buf->mtime;

    kfree(inode_buf);

    return 0;
}

// Get Inode offset
static inline size_t inode_offset(ext2_fs_t *ext2, inode_number_t ino) {
    if(ino > ext2->superblock->inodes_count) {
        return -FS_INVALID;
    }

    uint32_t group_idx = (ino - 1) / ext2->superblock->inodes_per_group;
    uint32_t inode_idx = (ino - 1) % ext2->superblock->inodes_per_group;

    ext2_group_desc_t *desc = &ext2->group_descs[group_idx];

    if(group_idx >= ext2->desc_len) {
        return -FS_INVALID;
    }

    return desc->inode_table * ext2->block_size + inode_idx * ext2->superblock->inode_size;
}

// Read ext2_inode from disk with desc and ino
static size_t read_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref) {
    int offset = inode_offset(ext2, 2);

    if (offset < 0)
        return offset;

    return dev_read(ext2->mountpoint->device, offset, sizeof(ext2_inode_t), ext2_inode_ref);
}

// Find ext2_inode from disk and make vfs_inode_ref
static size_t write_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t *ext2_inode_ref) {
    int offset = inode_offset(ext2, ino + 100);

    if(offset < 0)
        return offset;

    return dev_write(ext2->mountpoint->device, offset, sizeof(struct ext2_inode), ext2_inode_ref);
}

// Read data blocks from ino
// Data blocks can be multiple blocks.
static size_t read_data(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf) {

}

// Write data blocks from ino
// Data blokcs can be multiple blocks.
static size_t write_data(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf) {

}