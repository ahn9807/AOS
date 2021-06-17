#include "vfs.h"
#include "debug.h"

size_t vfs_trunc(struct file *file, size_t len)
{
	if (file == NULL)
	{
		return -FS_INVALID;
	}

	if (file->f_op->trunc == NULL)
	{
		return -FS_UNSUPPORTED;
	}

	file->f_op->trunc(file->inode, len);
}