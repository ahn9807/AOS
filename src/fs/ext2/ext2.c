#include "ext2.h"
#include "vfs.h"
#include "kmalloc.h"
#include "vga_text.h"
#include "debug.h"
#include "stat.h"
#include "string.h"
#include "bitmap.h"
#include "pmm.h"

static int ext2_mount(device_t *device, inode_t *super_node, void *aux);
static size_t ext2_truncate(inode_t *inode, size_t len);
static size_t ext2_read(inode_t *inode, void *buf, size_t len, size_t offset);
static size_t ext2_write(inode_t *inode, void *buf, size_t len, size_t offset);
static int ext2_readdir(inode_t *inode, size_t offset, dentry_t *dir);

static int sync_inode(ext2_fs_t *ext2, ext2_inode_t *ext2_inode, inode_t *inode);
static int sync_gdesc(ext2_fs_t *ext2);
static int sync_superblock(ext2_fs_t *ext2);

static int build_vfs_inode(ext2_fs_t *ext2, inode_number_t inode_nr, ext2_inode_t *ext2_inode, inode_t *inode);
static int build_ext2_inode(ext2_fs_t *ext2, inode_t *inode, ext2_inode_t *ext2_inode);
static inline size_t inode_offset(ext2_fs_t *ext2, inode_number_t ino);
static size_t read_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref);
static size_t write_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref);
static int alloc_inode(ext2_fs_t *ext2);
static int free_inode(ext2_fs_t *ext2, uint32_t inode_ptr);

static uint32_t block_offset(ext2_fs_t *ext2, ext2_inode_t *inode, uint32_t block_idx);
static size_t read_block(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf);
static size_t write_block(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf);
static int write_block_with_alloc(ext2_fs_t *ext2, ext2_inode_t *inode, size_t inode_nr, size_t block_idx, void *buf);
static int alloc_block(ext2_fs_t *ext2);
static int free_block(ext2_fs_t *ext2, uint32_t block_ptr);

struct fs_operations vfs_ext2_fs_operations = {
    .mount = &ext2_mount,
};

struct file_operations vfs_ext2_file_operations = {
    .read = &ext2_read,
    .write = &ext2_write,
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
int ext2_init()
{
    vfs_install(&vfs_ext2);
}

// Read data from file
// File must contain valid inode
static size_t ext2_read(inode_t *inode, void *buf, size_t size, size_t offset)
{
    ext2_inode_t *ext2_inode = kmalloc(sizeof(ext2_inode_t));
    ext2_fs_t *ext2 = inode->file_system;
    char *data_buf = kcalloc(1, ext2->block_size);

    read_inode(ext2, inode->inode_nr, ext2_inode);

    ASSERT(ext2_inode != NULL);

    if (offset > ext2_inode->size)
    {
        return 0;
    }

    size = size > ext2_inode->size - offset ? ext2_inode->size - offset : size;

    uint32_t start_block_idx = offset / ext2->block_size;
    uint32_t end_block_idx = (offset + size) / ext2->block_size;
    uint32_t start_block_offset = offset % ext2->block_size;
    uint32_t end_block_offset = (offset + size) % ext2->block_size;
    uint32_t cur_buf_offset = 0;

    for (int idx = start_block_idx; idx <= end_block_idx; idx++)
    {
        uint32_t left = 0;
        uint32_t right = ext2->block_size - 1;
        read_block(ext2, ext2_inode, idx, data_buf);
        if (idx == start_block_idx)
        {
            left = start_block_offset;
        }
        if (idx == end_block_idx)
        {
            right = end_block_offset - 1;
        }
        memcpy((char *)buf + cur_buf_offset, data_buf + left, (right - left + 1));
        cur_buf_offset += (right - left + 1);
    }

    kfree(ext2_inode);
    kfree(data_buf);

    return size;
}

static size_t ext2_write(inode_t *inode, void *buf, size_t size, size_t offset)
{
    ext2_inode_t *ext2_inode = kmalloc(sizeof(ext2_inode_t));
    ext2_fs_t *ext2 = inode->file_system;
    char *data_buf = kcalloc(1, ext2->block_size);

    read_inode(ext2, inode->inode_nr, ext2_inode);

    ASSERT(ext2_inode != NULL);

    if (offset + size > ext2_inode->size)
    {
        inode->size = offset + size;
        ext2_inode->size = offset + size;
    }

    size = size > ext2_inode->size - offset ? ext2_inode->size - offset : size;

    uint32_t start_block_idx = offset / ext2->block_size;
    uint32_t end_block_idx = (offset + size) / ext2->block_size;
    uint32_t start_block_offset = offset % ext2->block_size;
    uint32_t end_block_offset = (offset + size) % ext2->block_size;
    uint32_t cur_buf_offset = 0;

    for (int idx = start_block_idx; idx <= end_block_idx; idx++)
    {
        uint32_t left = 0;
        uint32_t right = ext2->block_size - 1;
        read_block(ext2, ext2_inode, idx, data_buf);
        if (idx == start_block_idx)
        {
            left = start_block_offset;
        }
        if (idx == end_block_idx)
        {
            right = end_block_offset - 1;
        }
        memcpy(data_buf + left, (char *)buf + cur_buf_offset, (right - left + 1));
        write_block_with_alloc(ext2, ext2_inode, inode->inode_nr, idx, data_buf);
        cur_buf_offset += (right - left + 1);
    }

    kfree(ext2_inode);
    kfree(data_buf);

    return size;
}

// Read offset'th directory or file from this ext2
// return dentry objects contains name and inode for accessing file
// Have to change dentry to file to utilze the return struct dentry.
static int ext2_readdir(inode_t *inode, size_t offset, dentry_t *dir)
{
    ext2_inode_t *ext2_inode = kmalloc(1024);
    ext2_fs_t *ext2 = (ext2_fs_t *)inode->file_system;
    read_inode(inode->file_system, inode->inode_nr, ext2_inode);

    if (dir == NULL || inode == NULL || ext2_inode == NULL)
    {
        kfree(ext2_inode);
        return -FS_INVALID;
    }
    else if (!S_ISDIR(inode->type) || !S_ISDIR(ext2_inode->mode))
    {
        kfree(ext2_inode);
        return -FS_NOT_DIRECTORY;
    }

    uint32_t block_number = ext2_inode->size / ext2->block_size;
    uint32_t cur_offset = 0;
    void *buf = kmalloc(ext2->block_size);
    
    for (size_t i = 0; i < block_number; i++)
    {
        read_block(ext2, ext2_inode, i, buf);
        ext2_dentry_t *ext2_dentry = buf;
        while ((char *)ext2_dentry < (char *)buf + ext2->block_size)
        {
            if (offset == cur_offset)
            {
                dir->name = kmalloc(ext2_dentry->name_len);
                dir->inode = kmalloc(sizeof(inode_t));
                ext2_inode_t *child_ext2_inode = kmalloc(sizeof(ext2_inode_t));
                
                dir->d_op = &vfs_ext2_dentry_operations;
                read_inode(ext2, ext2_dentry->inode_nr, child_ext2_inode);
                build_vfs_inode(ext2, ext2_dentry->inode_nr, child_ext2_inode, dir->inode);
                dir->inode->file_system = ext2;
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

    kfree(ext2_inode);
    kfree(buf);

    return -FS_NO_ENTRY;
}

// Create ext2 file or directory entry at inode
// Inode must be a type directory
static size_t ext2_mknod(inode_t *dir_node, char* name, uint16_t mode, uint16_t permission)
{
    PANIC("NOT IMPLEMENTED");
    int err = 0;

    ext2_fs_t *desc = dir_node->file_system;
    ext2_inode_t dir_inode;
    
    if ((err = ext2_inode_read(desc, dir_node->inode_nr, &dir_inode))) {
        /* TODO Error checking */
    }

    // /* file exists */
    // if (ext2_dentry_find(desc, &dir_inode, fn))
    //     return -FS_EXIST;

    uint32_t ino = alloc_inode(desc);
    struct ext2_inode inode;
    
    if ((err = read_inode(desc, ino, &inode))) {
        /* TODO Error checking */
    }

    memset(&inode, 0, sizeof(struct ext2_inode));

    inode.mode = permission | mode;

    // struct timespec ts;
    // gettime(&ts);

    inode.ctime = 0;
    inode.atime = 0;
    inode.mtime = 0;

    if (S_ISDIR(mode)) {   /* Initalize directory structure */
        inode.links_count = 2;
        inode.size = desc->block_size;
        
        char *buf = kmalloc(desc->block_size);
        struct ext2_dentry *d = (struct ext2_dentry *) buf;
        d->inode_nr = ino;
        d->rec_len = 12;
        d->name_len = 1;
        d->file_type = EXT2_S_IFDIR ;
        memcpy(d->name, ".", 1);
        d = (struct ext2_dentry *) ((char *) d + 12);
        d->inode_nr = dir_node->inode_nr;
        d->rec_len = desc->block_size - 12;
        d->name_len = 2;
        d->file_type = EXT2_S_IFDIR;
        memcpy(d->name, "..", 2);
        write_block_with_alloc(desc, &inode, ino, 0, buf);
        kfree(buf);
    } else {
        inode.links_count = 1;
        write_inode(desc, ino, &inode);
    }

    // ext2_dentry_create(dir_node, fn, ino, inode.mode);

    return 0;
}

static size_t ext2_rmnod(inode_t *dir_node, char *name) {
    PANIC("NOT IMPLMENTED");
}

static size_t ext2_truncate(inode_t *inode, size_t len)
{
    ext2_inode_t *ext2_inode = kmalloc(sizeof(ext2_inode_t));
    read_inode(inode->file_system, inode->inode_nr, ext2_inode);

    if (len == inode->size)
    {
        kfree(ext2_inode);
        return 0;
    }
    else if (len > inode->size)
    {
        // Allocate blocks
        inode->size = len;
        sync_inode((ext2_fs_t *)inode->file_system, ext2_inode, inode);
        kfree(ext2_inode);
        return len;
    }
    else
    {
        // Free blocks
        inode->size = len;
        sync_inode((ext2_fs_t *)inode->file_system, ext2_inode, inode);
        kfree(ext2_inode);
        return len;
    }
}

// Mount ext2 filesystem.
// This return valid super_node for accessing entire disk.e
static int ext2_mount(device_t *device, struct inode *super_node, void *aux)
{
    ext2_superblock_t *sb = kmalloc(sizeof(ext2_superblock_t));
    ext2_fs_t *fs = kmalloc(sizeof(ext2_fs_t));

    // Read super block
    dev_read(device, 1024, sizeof(ext2_superblock_t), sb);

    if (sb->magic != EXT2_MAGIC)
    {
        return -FS_INVALID;
    }

    // Set filesystem and return aux
    fs->superblock = sb;
    fs->block_size = 1024UL << sb->log_block_size;
    fs->desc_len = (sb->blocks_count + sb->blocks_per_group - 1) / sb->blocks_per_group;
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
static int build_vfs_inode(ext2_fs_t *ext2, inode_number_t inode_nr, ext2_inode_t *ext2_inode, inode_t *inode)
{
    // read inode from disk / cache
    inode->device = ext2->mountpoint->device;
    inode->file_system = ext2;

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
static int build_ext2_inode(ext2_fs_t *ext2, inode_t *inode, ext2_inode_t *ext2_inode)
{
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
static int sync_inode(ext2_fs_t *ext2, ext2_inode_t *ext2_inode, inode_t *inode)
{
    build_ext2_inode(ext2, inode, ext2_inode);
    write_inode(ext2, inode->inode_nr, ext2_inode);
}

// Sync block group descriptor
static int sync_gdesc(ext2_fs_t *ext2)
{
    dev_write(ext2->mountpoint->device, ext2->block_size == 1024 ? 2048 : ext2->block_size, ext2->desc_len * sizeof(ext2_group_desc_t), ext2->group_descs);
    return 0;
}

// Sync superblock
static int sync_superblock(ext2_fs_t *ext2)
{
    dev_write(ext2->mountpoint->device, 1024, sizeof(ext2_superblock_t), ext2->superblock);
    return 0;
}

// Get Inode offset
static inline size_t inode_offset(ext2_fs_t *ext2, inode_number_t ino)
{
    if (ino > ext2->superblock->inodes_count)
    {
        return -FS_INVALID;
    }

    uint32_t group_idx = (ino - 1) / ext2->superblock->inodes_per_group;
    uint32_t inode_idx = (ino - 1) % ext2->superblock->inodes_per_group;

    ext2_group_desc_t *desc = &ext2->group_descs[group_idx];

    if (group_idx >= ext2->desc_len)
    {
        return -FS_INVALID;
    }

    return desc->inode_table * ext2->block_size + inode_idx * ext2->superblock->inode_size;
}

// Read ext2_inode from disk with desc and ino
static size_t read_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref)
{
    int offset = inode_offset(ext2, ino);

    if (offset <= 0)
        return offset;

    return dev_read(ext2->mountpoint->device, offset, sizeof(ext2_inode_t), ext2_inode_ref);
}

// Find ext2_inode from disk and make vfs_inode_ref
static size_t write_inode(ext2_fs_t *ext2, inode_number_t ino, ext2_inode_t *ext2_inode_ref)
{
    int offset = inode_offset(ext2, ino);

    if (offset <= 0)
        return offset;

    return dev_write(ext2->mountpoint->device, offset, sizeof(ext2_inode_t), ext2_inode_ref);
}

// Get Data block number
static uint32_t block_offset(ext2_fs_t *ext2, ext2_inode_t *inode, uint32_t block_idx)
{
    uint32_t p = ext2->block_size / 4; // 4bytes per block pointer.
    uint32_t ret;
    int a, b, c, d, e, f, g;
    uint32_t block_nr = (inode->size + ext2->block_size + 1) / ext2->block_size;

    ASSERT(block_idx < block_nr);

    a = block_idx - EXT2_DIRECT_BLOCKS;

    if (a < 0)
    {
        return inode->block[block_idx];
    }
    uint32_t *tmp = kmalloc(ext2->block_size); // size of block pointer is 4bytes
    b = a - p;
    if (b < 0)
    {
        dev_read(ext2->mountpoint->device, SINGLE_INDIRECT_POINTER(inode), ext2->block_size, tmp);
        ret = tmp[a];
        goto done;
    }
    c = b - p * p;
    if (c < 0)
    {
        c = b / p;
        d = b - c * p;
        dev_read(ext2->mountpoint->device, DOUBLE_INDIRECT_POINTER(inode), ext2->block_size, tmp);
        dev_read(ext2->mountpoint->device, tmp[c], ext2->block_size, tmp);
        ret = tmp[d];
        goto done;
    }
    d = c - p * p * p;
    if (d < 0)
    {
        e = c / (p * p);
        f = (c - e * p * p) / p;
        g = (c - e * p * p - f * p);
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
static size_t read_block(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf)
{
    uint32_t block_idx = block_offset(ext2, inode, idx);
    return dev_read(ext2->mountpoint->device, block_idx * ext2->block_size, ext2->block_size, buf);
}

// Write data blocks from inode and index
// Data blokcs can be multiple blocks.
static size_t write_block(ext2_fs_t *ext2, ext2_inode_t *inode, size_t idx, void *buf)
{
    uint32_t block_idx = block_offset(ext2, inode, idx);
    return dev_write(ext2->mountpoint->device, block_idx * ext2->block_size, ext2->block_size, buf);
}

/* Allocate inode */
static int alloc_inode(ext2_fs_t *ext2)
{
    struct bitmap *imap = bitmap_create(ext2->block_size * 8);

    uint32_t group_idx = 0;
    uint32_t inode_idx = 0;
    uint32_t inode_nr = 0;

    for (int i = 0; i < ext2->desc_len; i++)
    {
        if (ext2->group_descs[i].free_inodes_count > 0)
        {
            dev_read(ext2->mountpoint->device, ext2->group_descs[i].inode_bitmap, ext2->block_size, bitmap_get_raw(imap));
            bitmap_set_from_buf(imap, ext2->block_size, bitmap_get_raw(imap));

            if ((inode_idx = bitmap_find(imap, 0)) != -1)
            {
                inode_nr = inode_idx + group_idx * ext2->superblock->inodes_per_group + 1;
                group_idx = i;
                break;
            }
        }
    }

    if (inode_nr == 0)
    {
        bitmap_destroy(imap);
        return 0;
    }

    bitmap_mark(imap, inode_idx);
    dev_write(ext2->mountpoint->device, ext2->group_descs[group_idx].inode_bitmap, ext2->block_size, bitmap_get_raw(imap));

    ext2->group_descs[group_idx].free_inodes_count--;
    sync_gdesc(ext2);

    ext2->superblock->free_inodes_count--;
    sync_superblock(ext2);

    bitmap_destroy(imap);

    return inode_nr;
}

/* Deallocate inode */
static int free_inode(ext2_fs_t *ext2, uint32_t inode_ptr)
{
    struct bitmap *imap = bitmap_create(ext2->block_size * 8);

    uint32_t group_idx = (inode_ptr - 1) % ext2->superblock->blocks_per_group;
    uint32_t inode_idx = (inode_ptr - 1) - group_idx * ext2->superblock->blocks_per_group;

    dev_read(ext2->mountpoint->device, ext2->group_descs[group_idx].inode_bitmap, ext2->block_size, bitmap_get_raw(imap));

    bitmap_reset(imap, inode_idx);
    dev_write(ext2->mountpoint->device, ext2->group_descs[group_idx].inode_bitmap, ext2->block_size, bitmap_get_raw(imap));

    ext2->group_descs[group_idx].free_inodes_count++;
    sync_gdesc(ext2);

    ext2->superblock->free_inodes_count++;
    sync_superblock(ext2);

    bitmap_destroy(imap);

    return 0;
}

/* Allocate block */
static int alloc_block(ext2_fs_t *ext2)
{
    struct bitmap *bmap = bitmap_create(ext2->block_size * 8);

    uint32_t group_idx = 0;
    uint32_t block_idx = 0;
    uint32_t block_nr = 0;

    for (int i = 0; i < ext2->desc_len; i++)
    {
        if (ext2->group_descs[i].free_blocks_count > 0)
        {
            dev_read(ext2->mountpoint->device, ext2->group_descs[i].block_bitmap, ext2->block_size, bitmap_get_raw(bmap));
            bitmap_set_from_buf(bmap, ext2->block_size, bitmap_get_raw(bmap));

            if ((block_idx = bitmap_find(bmap, 0)) != -1)
            {
                block_nr = block_idx + group_idx * ext2->superblock->blocks_per_group;
                group_idx = i;
                break;
            }
        }
    }

    if (block_nr == 0)
    {
        bitmap_destroy(bmap);
        return 0;
    }

    bitmap_mark(bmap, block_idx);
    dev_write(ext2->mountpoint->device, ext2->group_descs[group_idx].block_bitmap, ext2->block_size, bitmap_get_raw(bmap));

    ext2->group_descs[group_idx].free_blocks_count--;
    sync_gdesc(ext2);

    ext2->superblock->free_blocks_count--;
    sync_superblock(ext2);

    bitmap_destroy(bmap);

    return block_nr;
}

/* Deallocate block */
static int free_block(ext2_fs_t *ext2, uint32_t block_ptr)
{
    struct bitmap *bmap = bitmap_create(ext2->block_size * 8);

    uint32_t group_idx = block_ptr % ext2->superblock->blocks_per_group;
    uint32_t block_idx = block_ptr - group_idx * ext2->superblock->blocks_per_group;

    dev_read(ext2->mountpoint->device, ext2->group_descs[group_idx].block_bitmap, ext2->block_size, bitmap_get_raw(bmap));

    bitmap_reset(bmap, block_idx);
    dev_write(ext2->mountpoint->device, ext2->group_descs[group_idx].block_bitmap, ext2->block_size, bitmap_get_raw(bmap));

    ext2->group_descs[group_idx].free_blocks_count++;
    sync_gdesc(ext2);

    ext2->superblock->free_blocks_count++;
    sync_superblock(ext2);

    bitmap_destroy(bmap);

    return 0;
}

/* Write data to free blocks and update metadatas */
static int write_block_with_alloc(ext2_fs_t *ext2, ext2_inode_t *inode, size_t inode_nr, size_t block_idx, void *buf)
{
    uint32_t p = ext2->block_size / 4;
    uint32_t ret;

    if (block_idx < EXT2_DIRECT_BLOCKS)
    {
        if (!inode->block[block_idx])
        {
            inode->block[block_idx] = alloc_block(ext2);
            write_inode(ext2, inode_nr, inode);
        }
        write_block(ext2, inode, block_idx, buf);
    }
    else if (block_idx < EXT2_DIRECT_BLOCKS + p)
    {
        if (!SINGLE_INDIRECT_POINTER(inode))
        {
            SINGLE_INDIRECT_POINTER(inode) = alloc_block(ext2);
            write_inode(ext2, inode_nr, inode);
        }

        uint32_t *tmp = kcalloc(1, ext2->block_size);
        read_block(ext2, inode, block_idx, tmp);
        // 4 byte per block ptr
        uint32_t block = tmp[block_idx - EXT2_DIRECT_BLOCKS];

        if (!block)
        {
            tmp[block_idx - EXT2_DIRECT_BLOCKS] = alloc_block(ext2);
            write_block(ext2, inode, SINGLE_INDIRECT_POINTER(inode), tmp);
        }

        kfree(tmp);
        write_block(ext2, inode, block_idx, buf);
    }
    else
    {
        PANIC("NOT IMPLEMENTED");
    }

    return 0;
}

static int create_dentry(inode_t *dir_node, char *name, inode_number_t ino) {
    PANIC("NOT IMPLEMENTED");

    ext2_fs_t *ext2 = dir_node->file_system;

    size_t length = strlne(name);
    size_t d_size = length + sizeof(ext2_dentry_t);
    d_size = (d_size + 3) & ~3;

    char *buf = kcalloc(1, ext2->block_size);
    

}