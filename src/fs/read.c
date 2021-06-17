#include "vfs.h"

size_t vfs_read(struct file *file, void *buffer, size_t len, size_t offset)
{
	if (file == NULL || buffer == NULL)
	{
		return -FS_INVALID;
	}

	if (file->f_op->read == NULL)
	{
		return -FS_UNSUPPORTED;
	}

	file->f_op->read(file->inode, buffer, len, offset);
}