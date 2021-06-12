#include "ext2.h"
#include "vfs.h"
#include "kmalloc.h"
#include "vga_text.h"
#include "debug.h"
#include "stat.h"
#include "string.h"

static int mount(device_t *device, struct inode *ret, void *aux);
static int build_vfs_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t *ret);
static inline size_t inode_offset(ext2_fs_t *ext2, inode_number_t ino);
static size_t read_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref);
static size_t write_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t *ext2_inode_ref);
static int ext2_mount(device_t *device, inode_t *super_node, void *aux);
static uint32_t block_offset(ext2_fs_t *ext2, ext2_inode_t *inode, uint32_t block_idx);
static size_t read_data(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf);
static size_t write_data(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf);
static int ext2_readdir(inode_t *inode, size_t offset, dentry_t *dir);


struct fs_operations vfs_ext2_fs_operations = {
    .mount = &ext2_mount,
};

struct file_operations vfs_ext2_file_operations = {

};

struct dentry_operations vfs_ext2_dentry_operations = {

};

struct inode_operations vfs_ext2_inode_operations = {
    .readdir = &ext2_readdir,
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

static int ext2_readdir(inode_t *inode, size_t offset, dentry_t *dir) {
    ext2_inode_t *ext2_inode = kmalloc(sizeof(ext2_inode_t));
    ext2_fs_t *ext2 = (ext2_fs_t *)inode->file_system;
    read_inode(inode->file_system, inode->inode_num, ext2_inode);

    if(dir == NULL || inode == NULL || ext2_inode == NULL) {
        return -FS_INVALID;
    } else if(!S_ISDIR(inode->type) || !S_ISDIR(ext2_inode->mode)) {
        return -FS_NOT_DIRECTORY;
    }

    uint32_t block_number = ext2_inode->size / ext2->block_size;
    uint32_t cur_offset = 0;
    uint8_t *buf = kmalloc(ext2->block_size);

    for(size_t i = 0; i < block_number; i++) {
        read_data(ext2, ext2_inode, i, buf);
        ext2_dentry_t *ext2_dentry = buf;
        while((char *)ext2_dentry < (char *)buf + ext2->block_size) {
            if(offset == cur_offset) {
                dir->inode_nr = ext2_dentry->inode_nr;
                dir->name = kmalloc(ext2_dentry->name_len);
                dir->d_op = &vfs_ext2_dentry_operations;
                memcpy(dir->name, (char *)ext2_dentry->name, ext2_dentry->name_len);
                dir->name[ext2_dentry->name_len] = '\0';
                kfree(ext2_inode);
                kfree(buf);
                return 1;
            }
            ext2_dentry = (ext2_dentry_t *)((char *)ext2_dentry + ext2_dentry->rec_len);
            cur_offset++;
        }
    }

    kfree(ext2_inode);
    kfree(buf);
    return 0;
}

static size_t ext2_mkdir(inode_t *inode, dentry_t *dir) {

}

static size_t ext2_truncate(inode_t *inode, size_t len) {

}

static int mount(device_t *device, struct inode *super_node, void *aux) {
    ext2_superblock_t *sb = kmalloc(sizeof(ext2_superblock_t));
    ext2_fs_t *fs = kmalloc(sizeof(ext2_fs_t));

    // Read super block
    dev_read(device, 1024, sizeof(ext2_superblock_t), sb);

    if(sb->magic != EXT2_MAGIC) {
        return -FS_INVALID;
    }

    // Set filesystem and return aux
    fs->superblock = sb;
    fs->block_size = 1024UL << sb->log_block_size;
    fs->desc_len = (sb->blocks_count + sb->blocks_per_group - 1)/ sb->blocks_per_group;
    fs->group_descs = kcalloc(fs->desc_len, sizeof(ext2_group_desc_t));
    fs->mountpoint = super_node;
    fs->mountpoint->device = device;
    aux = fs;

    // Read group descs
    dev_read(device, fs->block_size == 1024 ? 2048 : fs->block_size, fs->desc_len * sizeof(ext2_group_desc_t), fs->group_descs);

    build_vfs_inode(fs, 2, super_node);
}

static int build_vfs_inode(ext2_fs_t *ext2, inode_number_t ino, inode_t *inode) {
    // read inode from disk / cache
    ext2_inode_t *inode_buf = kcalloc(1, sizeof(ext2_inode_t));
    inode->device = ext2->mountpoint->device;
    inode->file_system = (void*)ext2;
    read_inode(ext2, ino, inode_buf);

    inode->inode_num = ino;
    inode->size = inode_buf->size;
    inode->type = inode_buf->mode & 0xFF00;
    inode->permission = inode_buf->mode & 0x00FF;
    inode->uid = inode_buf->uid;
    inode->gid = inode_buf->gid;
    inode->nlink = inode_buf->links_count;

    inode->atime = inode_buf->atime;
    inode->ctime = inode_buf->ctime;
    inode->mtime = inode_buf->mtime;

    inode->i_op = &vfs_ext2_inode_operations;
    inode->i_fop = &vfs_ext2_file_operations;

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

// Get Data block number
static uint32_t block_offset(ext2_fs_t *ext2, ext2_inode_t *inode, uint32_t block_idx) {
    uint32_t p = ext2->block_size / 4; // 4bytes per block pointer.
    uint32_t ret;
    int a, b, c, d, e, f, g;
    uint32_t block_nr = (inode->size + ext2->block_size + 1) / ext2->block_size;
    if(block_nr <= block_idx) {
        PANIC("INALID ACCESS TO DATA BLOCKS");
    }

    a = block_idx - EXT2_DIRECT_BLOCKS;

    if(a < 0) {
        return inode->block[block_idx];
    }
    uint32_t *tmp = kmalloc(ext2->block_size); // size of block pointer is 4bytes
    b = a - p;
    if(b < 0) {
        dev_read(ext2->mountpoint->device, SINGLE_INDIRECT_POINTER(inode), ext2->block_size, tmp);
        ret = tmp[a];
        goto done;
    }
    c = b - p * p;
    if(c < 0) {
        c = b / p;
        d = b - c * p;
        dev_read(ext2->mountpoint->device, DOUBLE_INDIRECT_POINTER(inode), ext2->block_size, tmp);
        dev_read(ext2->mountpoint->device, tmp[c], ext2->block_size, tmp);
        ret = tmp[d];
        goto done;
    }
    d = c - p * p * p;
    if(d < 0) {
        e = c / (p * p);
        f = (c - e * p * p) / p;
        g = (c - e * p * p -f * p);
        dev_read(ext2->mountpoint->device, TRIPLE_INDIRECT_POINTER(inode), ext2->block_size, tmp);
        dev_read(ext2->mountpoint->device, tmp[e], ext2->block_size, tmp);
        dev_read(ext2->mountpoint->device, tmp[f], ext2->block_size, tmp);
        ret = tmp[p];
        goto done;
    }

done:
    kfree(tmp);
    return ret;
}

// Read data blocks from inode and index
// Data blocks can be multiple blocks.
static size_t read_data(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf) {
    uint32_t block_idx = block_offset(ext2, inode, idx);
    return dev_read(ext2->mountpoint->device, block_idx * ext2->block_size, ext2->block_size, buf);
}

// Write data blocks from inode and index
// Data blokcs can be multiple blocks.
static size_t write_data(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf) {
    uint32_t block_idx = block_offset(ext2, inode, idx);
    return dev_write(ext2->mountpoint->device, block_idx * ext2->block_size, ext2->block_size, buf);
}