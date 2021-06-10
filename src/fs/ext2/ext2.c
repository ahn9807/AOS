#include "ext2.h"
#include "vfs.h"
#include "kmalloc.h"
#include "vga_text.h"
#include "debug.h"

static int mount_ext2(device_t *device, struct inode *ret, void *aux);
static int build_vfs_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t *ret);
static inline size_t get_ext2_inode_offset(ext2_fs_t *ext2, inode_number_t ino);
static int read_ext2_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref);
static int make_ext2_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t **vfs_inode_ref);
static int ext2_mount(device_t *device, inode_t *super_node, void *aux);

struct fs_operations vfs_ext2_operations = {
    .mount = &ext2_mount,
};

struct vfs_fs vfs_ext2 = {
    .name = "ext2",
    .fs_op = &vfs_ext2_operations,
};

int ext2_init() {
    vfs_install(&vfs_ext2);
}

static int ext2_mount(device_t *device, inode_t *super_node, void *aux) {
    mount_ext2(device, super_node, aux);
}

static int mount_ext2(device_t *device, struct inode *super_node, void *aux) {
    ext2_superblock_t *sb = kmalloc(sizeof(ext2_superblock_t));
    ext2_fs_t *fs = kmalloc(sizeof(ext2_fs_t));

    dev_read(device, 1024, sizeof(ext2_superblock_t), sb);

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
    if(read_ext2_inode(ext2, ino, inode_buf)) {
        PANIC("VFS PANIC");
    }

    inode->inode_num = ino;
    inode->size = inode_buf->size;
    inode->mask = inode_buf->mode;
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
static inline size_t get_ext2_inode_offset(ext2_fs_t *ext2, inode_number_t ino) {
    if(ino > ext2->superblock->inodes_count) {
        return -FS_INVALID;
    }

    uint32_t group_idx = (ino - 1) / ext2->superblock->inodes_per_group;
    uint32_t inode_idx = (ino - 1) % ext2->superblock->inodes_per_group;

    ext2_group_desc_t *desc = &ext2->group_descs[group_idx];

    if(group_idx > ext2->desc_len) {
        return -FS_INVALID;
    }

    // Why I have to do like this?
    // Not desc->inode_table * ext2->block_size + inode_idx * ext2->superblock->inode_size
    return (desc->inode_table + inode_idx * ext2->superblock->inode_size / ext2->block_size) * ext2->block_size;
}

// Read ext2_inode from disk with desc and ino
static int read_ext2_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref) {
    int offset = get_ext2_inode_offset(ext2, ino);

    if (offset < 0)
        return offset;

    if (ext2_inode_ref) {
        size_t size = sizeof(struct ext2_inode);
        size_t r;

        if ((r = dev_read(ext2->mountpoint->device, offset, size, ext2_inode_ref)) != size) {
            if (r < 0)
                return r;

            /* FIXME Check for further errors? */
            return -FS_INVALID;
        }
    }

    return 0;
}

// Find ext2_inode from disk and make vfs_inode_ref
static int make_ext2_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t **vfs_inode_ref) {

    // read_ext2_inode(desc, )
}