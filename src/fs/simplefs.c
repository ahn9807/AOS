#include "lib/debug.h"
#include "vfs.h"

int simple_fs_open(struct file *file)
{
	ASSERT(file->inode->refcount != 0);

	if (file->inode == NULL) {
		return -FS_INVALID;
	} else {
		file->offset = 0;
	}
}

uint64_t simple_fs_lseek(struct file *file, size_t offset, int whence)
{
	if (whence == SEEK_SET) {
		file->offset = offset;
	} else if (whence == SEEK_CUR) {
		file->offset += offset;
	} else if (whence == SEEK_END) {
		file->offset -= offset;
	}

	return file->offset;
}

uint64_t simple_fs_read(struct file *file, void *buffer, size_t size)
{
	PANIC("NOT IMPLEMENTED");
}

uint64_t simple_fs_write(struct file *file, void *buffer, size_t size)
{
	PANIC("NOT IMPLEMENTED");
}