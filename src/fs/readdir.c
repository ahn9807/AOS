#include "lib/debug.h"
#include "lib/string.h"
#include "stat.h"
#include "vfs.h"
#include "vga_text.h"

int vfs_readdir(struct inode *p_dir, size_t offset, struct dentry *dir)
{
	if (p_dir == NULL || dir == NULL) {
		return -FS_INVALID;
	}

	if (p_dir->i_op->readdir == NULL) {
		return -FS_UNSUPPORTED;
	}

	if (!S_ISDIR(p_dir->type)) {
		return -FS_NOT_DIRECTORY;
	}

	return p_dir->i_op->readdir(p_dir, offset, dir);
}