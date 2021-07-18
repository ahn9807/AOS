#include "vfs.h"
#include "printf.h"

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

	if(vfs_get_size(file) < file->offset + size) {
		size = vfs_get_size(file) - file->offset;
		if(size <= 0) {
			return 0;
		}
	}
	file->offset += size;
	return file->f_op->read(file->inode, buffer, size, file->offset - size);
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

	file->offset += size;
	return file->f_op->write(file->inode, buffer, size, file->offset - size);
}