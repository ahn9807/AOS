#include "vfs.h"
#include "stat.h"

int vfs_stat(struct inode* inode, struct stat* stat) {
    stat->st_dev   = 0;
    stat->st_ino   = inode->inode_nr;
    stat->st_mode  = inode->type | inode->permission;
    stat->st_nlink = inode->nlink;
    stat->st_uid   = inode->uid;
    stat->st_gid   = inode->gid;
    stat->st_rdev  = 0;
    stat->st_size  = inode->size;
    stat->st_mtime = inode->mtime;
    stat->st_atime = inode->atime;
    stat->st_ctime = inode->ctime;

	return 0;
}

size_t vfs_get_size(struct file* file) {
	return file->inode->size;
}