#pragma once

#include <stddef.h>
#include <stdint.h>
#include "list.h"
#include "spin_lock.h"
#include "device.h"

#define PATH_SEPARATOR '/'
#define PATH_SEPARATOR_STRING "/"
#define PATH_CHILD  ".."
#define PATH_CUR "."

// File system state is invalid
#define FS_INVALID 0
// IO Error
#define FS_IO_ERROR 1
// No such entry in file system
#define FS_NO_ENTRY 2
// Target is not a file
#define FS_NOT_FILE 3
// Target is not a directory
#define FS_NOT_DIRECTORY 4
// Directory is not emptry
#define FS_NOT_EMPTY 5
// File Exist
#define FS_EXIST 6
// File system is full
#define FS_FULL 7
// File system is shutdowned
#define FS_SHUTDOWN 8
// Unsupported
#define FS_UNSUPPORTED 9
// File is busy
#define FS_BUSY 10
// File system request is pending
#define FS_PENDING 11


#define O_RDONLY     0x0000
#define O_WRONLY     0x0001
#define O_RDWR       0x0002
#define O_APPEND     0x0008
#define O_CREAT      0x0200
#define O_TRUNC      0x0400
#define O_EXCL       0x0800
#define O_NOFOLLOW   0x1000
#define O_PATH       0x2000

#define FS_FILE        0x01
#define FS_DIRECTORY   0x02
#define FS_CHARDEVICE  0x04
#define FS_BLOCKDEVICE 0x08
#define FS_PIPE        0x10
#define FS_SYMLINK     0x20
#define FS_MOUNTPOINT  0x40

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

typedef uint64_t inode_number_t;

int path_search(const char *path, const char *s);
char **path_tokenize(const char *path);
char *path_absolute(char *cwd, char *path);
int path_length(char **path);

struct inode_operations;
struct file_operations;
typedef struct inode {
    device_t *device; /* Real device for this inode */
    void *file_system; /* Real filesystem for this inode */
    uint32_t permission; /* Access Permission */
    uint32_t uid;
    uint32_t gid;
    uint32_t type; /* Inode type (char, directory, file ...) */
    uint32_t inode_num;
    uint32_t size; /* Size in bytes */
    uint32_t fs_type; /* file system types */
    uint32_t ctime;
    uint32_t atime;
    uint32_t mtime;
    uint32_t offset;
    uint32_t nlink;
    int refcount;

    spinlock_t lock;

    struct inode_operations *i_op;
    struct file_operations *i_fop;
} inode_t;

// File system operations which mainly uses inode_t
// read_inode or write_inode should be implemented in each file systems
struct inode_operations {
    // called by open(2) and create(2) system call.
    int (*create)(struct inode *inode, char *name, uint16_t permission);
    // loopup an inode in a parent directory; return size fo dentries (number of directory)
    size_t (*readdir)(struct inode*, size_t offset, struct dentry*);
    // link(2) system call. Hard link
    int (*link)(struct inode *);
    // symlink(2) system call.
    int (*symlink)(struct inode *, struct dentry *, const char*);
    // unlink(2) system call. remove inode.
    int (*unlink)(struct inode *, const char *);
    // this method returns the symlink body to traverse.
    char * (*get_link)(struct inode *, struct dentry *);
    // make directory
    void (*mkdir)(struct inode *, char *name, uint16_t permission);
    // remove directory
    int (*rmdir)(struct inode *, struct dentry *);
    // get permission
    int (*permission)(struct inode *, int);
    // set time of the file
    void (*update_time)(struct inode *, /* struct timespec *,*/ int);
    // Change attribute of the inode
    int (*chmod)(struct inode *, uint32_t mode);
    // list all direcotry name inside this inode
    char ** (*listdir)(struct inode *);
};

struct dentry_operations;
typedef struct dentry {
    inode_number_t inode_nr;
    char* name;

    struct dentry_operations *d_op;
} dentry_t;

struct dentry_operations {

};

struct file_operations;
struct file {
    char name[256];
    struct inode* inode;
    uint64_t offset;
    spinlock_t lock;
    struct file_operations *f_op;
};

struct file_operations {
    // called by the VFS when an inode should be opened. When the VFS opens a file, it creates a new “struct file”
    int (*open)(struct inode *inode, struct file *file);
    // called when the VFS needs to move the file position index
    uint64_t (*lseek)(struct file *file, size_t offset, int whence);
    // called by read(2) and related system calls
    uint64_t (*read)(struct file *file, void *buf, size_t size);
    // called by write(2) and related system calls
    uint64_t (*write)(struct file *file, void *buf, size_t size);
    // called by the close(2) system call to flush a file
    int (*close)(struct file *);
    // called by the mmap(2) system call
    int (*mmap)(struct file *);
    // called by the ioctl(2) system call.
    int (*ioctl)(struct inode* inode, int request, void *argp);
    // called by the fsync(2) system call.
    int (*fsync)(struct file *);
    // trucated the file size
    int (*trunc)(struct file *file, size_t len);
};

// Represent file system
struct fs_operations {
    int (*init)();
    int (*mount)(device_t *device, inode_t *super_node, void *aux);
};

struct vfs_fs {
    char name[256];

    struct fs_operations *fs_op;
    struct list_elem elem;
};

struct vfs_node {
    char *name;
    struct vfs_node *children;
    struct vfs_node *next;

    struct vfs_fs *fs;
    struct inode *inode;
    //file system private data
    void *aux;
};

extern struct inode *vfs_root;
extern struct list fs_list;

/* VFS meta Functions */
void vfs_init();
void vfs_mount(const char* path, struct inode* local_root);
struct vfs_node *vfs_mountpoint(char *path);
int vfs_bind(const char *path, struct inode *target);
int vfs_install(struct vfs_fs *fs);
struct vfs_fs *vfs_find(char *name);

/* VFS Functions */
int vfs_open(struct inode *inode, struct file *file);
size_t vfs_read(struct file *file, size_t offset, size_t len, void *buffer);
size_t vfs_write(struct inode *inode, size_t offset, size_t len, void *buffer);
int vfs_readdir(struct inode* p_dir, size_t offset, struct dentry *dir);

// uint32_t vfs_write(struct inode *node, uint32_t offset, uint32_t size, char *buffer);
// void vfs_open(struct vfs_node *node, uint32_t flags);
// void vfs_close(struct inode *node);
// struct inode *vfs_finddir(struct inode *node, char *name);
// void vfs_mkdir(char *name, uint16_t permission);
// void vfs_mkfile(char *name, uint16_t permission);
// int vfs_create_file(char *name, uint16_t permission);
// struct inode *file_open(const char *file_name, uint32_t flags);
// char *expand_path(char *input);
// int vfs_ioctl(struct inode *node, int request, void * argp);
// void vfs_chmod(struct inode *node, uint32_t mode);
// void vfs_unlink(char * name);
// int vfs_symlink(char * value, char * name);
// int vfs_readlink(struct inode * node, char * buf, uint32_t size);
// void vfs_init();
// void vfs_mount(char * path, struct inode * local_root);
// typedef struct inode * (*vfs_mount_callback)(char * arg, char * mountpoint);
// void vfs_register(char * name, vfs_mount_callback callme);
// void vfs_mount_dev(char * mountpoint, struct inode * node);
// void print_vfstree();
// void vfs_db_listdir(char * name);

// Helper functions