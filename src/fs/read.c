#include "vfs.h"

size_t vfs_read(struct file *file, void *buffer, size_t size)
{
	if (file == NULL || buffer == NULL)
	{
		return -FS_INVALID;
	}

	if (file->f_op->read == NULL)
	{
		return -FS_UNSUPPORTED;
	}

	file->f_op->read(file->inode, buffer, size, file->offset);
}


size_t vfs_write(struct file *file, void *buffer, size_t size) {
	if (file == NULL || buffer == NULL)
	{
		return -FS_INVALID;
	}

	if (file->f_op->write == NULL)
	{
		return -FS_UNSUPPORTED;
	}

	file->f_op->write(file->inode, buffer, size, file->offset);
}