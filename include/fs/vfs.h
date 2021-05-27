#pragma once

#include <stddef.h>
#include <stdint.h>

enum vfs_error {
    // File system state is invalid
    FS_INVALID,
    // IO Error
    FS_IO_ERROR,
    // No such entry in file system
    FS_NO_ENTRY,
    // Target is not a file
    FS_NOT_FILE,
    // Target is not a directory
    FS_NOT_DIRECTORY,
    // Directory is not emptry
    FS_NOT_EMPTY,
    // File Exist
    FS_EXIST,
    // File system is full
    FS_FULL,
    // File system is shutdowned
    FS_SHUTDOWN,
    // Unsupported
    FS_UNSUPPORTED,
    // File is busy
    FS_BUSY,
    // File system request is pending
    FS_PENDING,
};

enum vfs_type {
    FS_UNKNOWN,
    FS_REGULAR,
    FS_DIRECTORY,
    FS_CHARACTERDEV,
    FS_BLOCKDEV,
    FS_PIPE,
    FS_SYMLINK,
    FS_MOUNTPOINT,
};

typedef uint64_t inode_number_t;

struct dirent {
    inode_number_t ino;
    char name[256];
};

struct vfs_entry {
    char *name;
}