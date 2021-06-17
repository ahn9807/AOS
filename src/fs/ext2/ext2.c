#include "ext2.h"
#include "vfs.h"
#include "kmalloc.h"
#include "vga_text.h"
#include "debug.h"
#include "stat.h"
#include "string.h"
#include "bitmap.h"

static int mount(device_t *device, struct inode *ret, void *aux);
static int build_vfs_inode(ext2_fs_t *ext2, inode_number_t inode_nr, ext2_inode_t *ext2_inode, inode_t *inode);
static int build_ext2_inode(ext2_fs_t *ext2, inode_t* inode, ext2_inode_t *ext2_inode);
static inline size_t inode_offset(ext2_fs_t *ext2, inode_number_t ino);
static size_t read_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref);
static size_t write_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref);
static int ext2_mount(device_t *device, inode_t *super_node, void *aux);
static uint32_t block_offset(ext2_fs_t *ext2, ext2_inode_t *inode, uint32_t block_idx);
static size_t read_data(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf);
static size_t write_data(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf);
static int ext2_readdir(inode_t *inode, size_t offset, dentry_t *dir);
static size_t ext2_read(inode_t *inode, void *buf, size_t len, size_t offset);
static int sync_inode(ext2_fs_t *ext2, ext2_inode_t *ext2_inode, inode_t *inode);
static int sync_gdesc(ext2_fs_t *ext2);
static int sync_superblock(ext2_fs_t *ext2);
static int allocate_block(ext2_fs_t *ext2);
static int free_block(ext2_fs_t *ext2, uint32_t block_ptr);
static size_t ext2_truncate(inode_t *inode, size_t len);
static int alloc_inode(ext2_fs_t *ext2);

struct fs_operations vfs_ext2_fs_operations = {
    .mount = &ext2_mount,
};

struct file_operations vfs_ext2_file_operations = {
    .read = &ext2_read,
    .trunc = &ext2_truncate,
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

// Initialize ext2 filesystem
int ext2_init() {
    vfs_install(&vfs_ext2);
}

// Mount ext2 filesystem.
// This return valid super_node for accessing entire disk.
static int ext2_mount(device_t *device, inode_t *super_node, void *aux) {
    mount(device, super_node, aux);
}

// Read data from file
// File must contain valid inode
static size_t ext2_read(inode_t *inode, void *buf, size_t len, size_t offset) {
    ext2_inode_t *ext2_inode = kmalloc(sizeof(ext2_inode_t));
    ext2_fs_t *ext2 = (ext2_fs_t *)inode->file_system;
    char* data_buf = kcalloc(1, ext2->block_size);

    ASSERT(ext2 != NULL);

    read_inode(ext2, inode->inode_nr, ext2_inode);

    ASSERT(ext2_inode != NULL);

    if(offset > ext2_inode->size) {
        return 0;
    }

    len = len > ext2_inode->size - offset ? ext2_inode->size - offset : len;

    uint32_t start_block_idx = offset / ext2->block_size;
    uint32_t end_block_idx = (offset + len) / ext2->block_size;
    uint32_t start_block_offset = offset % ext2->block_size;
    uint32_t end_block_offset = (offset + len) % ext2->block_size;
    uint32_t cur_buf_offset = 0;

    for(int idx = start_block_idx; idx <= end_block_idx; idx++) {
        uint32_t left = 0;
        uint32_t right = ext2->block_size - 1;
        read_data(ext2, ext2_inode, idx, data_buf);
        if(idx == start_block_idx) {
            left = start_block_offset;
        } else if(idx == end_block_idx) {
            right = end_block_offset - 1;
        }
        memcpy((char *)buf + cur_buf_offset, data_buf + left, (right - left + 1));
        cur_buf_offset += (right - left + 1);
    }

    kfree(ext2_inode);
    kfree(data_buf);

    return len;
}

static size_t ext2_write(inode_t *inode, size_t offset, size_t len, void *buf) {

}

// Read offset'th directory or file from this ext2
// return dentry objects contains name and inode for accessing file
// Have to change dentry to file to utilze the return struct dentry.
static int ext2_readdir(inode_t *inode, size_t offset, dentry_t *dir) {
    ext2_inode_t *ext2_inode = kmalloc(sizeof(ext2_inode_t));
    ext2_fs_t *ext2 = (ext2_fs_t *)inode->file_system;
    read_inode(inode->file_system, inode->inode_nr, ext2_inode);

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
                dir->name = kmalloc(ext2_dentry->name_len);
                dir->d_op = &vfs_ext2_dentry_operations;
                dir->inode = kmalloc(sizeof(inode_t));
                ext2_inode_t *child_ext2_inode = kmalloc(sizeof(ext2_inode_t));
                read_inode(ext2, ext2_dentry->inode_nr, child_ext2_inode);
                build_vfs_inode(ext2, ext2_dentry->inode_nr, child_ext2_inode, dir->inode);
                memcpy(dir->name, (char *)ext2_dentry->name, ext2_dentry->name_len);
                dir->name[ext2_dentry->name_len] = '\0';
                kfree(ext2_inode);
                kfree(buf);
                kfree(child_ext2_inode);
                return 1;
            }
            ext2_dentry = (ext2_dentry_t *)((char *)ext2_dentry + ext2_dentry->rec_len);
            cur_offset++;
        }
    }

    alloc_inode(ext2);

    kfree(ext2_inode);
    kfree(buf);

    return 0;
}

static size_t ext2_mkdir(inode_t *inode, dentry_t *dir) {

}

static size_t ext2_truncate(inode_t *inode, size_t len) {
    ext2_inode_t *ext2_inode = kmalloc(sizeof(ext2_inode_t));
    read_inode(inode->file_system, inode->inode_nr, ext2_inode);
    printf("trunc: %d\n", ext2_inode->mtime);

    if(len == inode->size) {
        kfree(ext2_inode);
        return 0;
    } else if(len > inode->size) {
        // Allocate blocks
        inode->size = len;
		sync_inode((ext2_fs_t *)inode->file_system, ext2_inode, inode);
        kfree(ext2_inode);
        return len;
    } else {
        // Free blocks
        inode->size = len;
		sync_inode((ext2_fs_t *)inode->file_system, ext2_inode, inode);
        kfree(ext2_inode);
        return len;
    }
}

// Mount ext2 to file system
// Return super_node for accessing entire file from device
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

    ext2_inode_t *ext2_inode = kmalloc(sizeof(ext2_inode_t));
    read_inode(fs, 2, ext2_inode);

    build_vfs_inode(fs, 2, ext2_inode, super_node);
    kfree(ext2_inode);
}

// Build vfs_inode from ext2_inode
static int build_vfs_inode(ext2_fs_t *ext2, inode_number_t inode_nr, ext2_inode_t *ext2_inode, inode_t *inode) {
    // read inode from disk / cache
    inode->device = ext2->mountpoint->device;
    inode->file_system = (void*)ext2;

    inode->inode_nr = inode_nr;
    inode->size = ext2_inode->size;
    inode->type = ext2_inode->mode & 0xFFF000;
    inode->permission = ext2_inode->mode & 0x000FFF;
    inode->uid = ext2_inode->uid;
    inode->gid = ext2_inode->gid;
    inode->nlink = ext2_inode->links_count;

    inode->atime = ext2_inode->atime;
    inode->ctime = ext2_inode->ctime;
    inode->mtime = ext2_inode->mtime;

    inode->i_op = &vfs_ext2_inode_operations;
    inode->i_fop = &vfs_ext2_file_operations;

    return 0;
}

// Build ext2_inode from vfs_inode
static int build_ext2_inode(ext2_fs_t *ext2, inode_t* inode, ext2_inode_t *ext2_inode) {
    ext2_inode->atime = inode->atime;
    ext2_inode->mtime = inode->mtime;
    ext2_inode->ctime = inode->ctime;
    ext2_inode->blocks = inode->size / ext2->block_size + 1;
    ext2_inode->size = inode->size;
    ext2_inode->mode = inode->type | inode->permission;
    ext2_inode->uid = inode->uid;
    ext2_inode->gid = inode->gid;
    ext2_inode->links_count = inode->nlink;

    return 0;
}

// Sync inode with ext2_inode
// This overwrite ext2_inode_t to metadata
static int sync_inode(ext2_fs_t *ext2, ext2_inode_t *ext2_inode, inode_t *inode) {
    build_ext2_inode(ext2, inode, ext2_inode);
	printf("w len2: %d\n", ext2_inode->size);
    write_inode(ext2, inode->inode_nr, ext2_inode);
    read_inode(ext2, inode->inode_nr, ext2_inode);
	printf("w len3: %d\n", ext2_inode->size);
}

// Sync block group descriptor
static int sync_gdesc(ext2_fs_t *ext2) {
    PANIC("NOT IMPLEMENTED");
    return 0;
}

// Sync superblock
static int sync_superblock(ext2_fs_t *ext2) {
    PANIC("NOT IMPLEMENTED");
    return 0;
}

// Allocate block
static int allocate_block(ext2_fs_t *ext2) {
    PANIC("NOT IMPLEMENTED");
    return 0;
}

// Deallocate block
static int free_block(ext2_fs_t *ext2, uint32_t block_ptr) {
    PANIC("NOT IMPLEMENTED");
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
    int offset = inode_offset(ext2, ino);

    if (offset <= 0)
        return offset;

    return dev_read(ext2->mountpoint->device, offset, sizeof(ext2_inode_t), ext2_inode_ref);
}

// Find ext2_inode from disk and make vfs_inode_ref
static size_t write_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref) {
    int offset = inode_offset(ext2, ino);

    if(offset <= 0)
        return offset;

    return dev_write(ext2->mountpoint->device, offset, sizeof(ext2_inode_t), ext2_inode_ref);
}

// Get Data block number
static uint32_t block_offset(ext2_fs_t *ext2, ext2_inode_t *inode, uint32_t block_idx) {
    uint32_t p = ext2->block_size / 4; // 4bytes per block pointer.
    uint32_t ret;
    int a, b, c, d, e, f, g;
    uint32_t block_nr = (inode->size + ext2->block_size + 1) / ext2->block_size;

    ASSERT(block_idx < block_nr);

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

static int alloc_inode(ext2_fs_t *ext2) {
    uint32_t *_imap = kmalloc(sizeof(ext2_group_desc_t));
    struct bitmap *imap;

    for(int i=0;i<ext2->desc_len; i++) {
        dev_read(ext2->mountpoint->device, ext2->group_descs[i].inode_bitmap, ext2->block_size, _imap);
        break;
    }
}

static int alloc_block(ext2_fs_t *ext2) {

}

static int write_data_with_alloc(ext2_fs_t *ext2, ext2_inode_t *inode, void *buf) {

}