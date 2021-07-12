#include "vfs.h"

size_t vfs_offset(struct file *file, off_t offset) {
	if(file == NULL) {
		return -FS_INVALID;
	}

	file->offset = offset;

	return file->offset;
}

size_t vfs_seek(struct file *file, off_t offset, int whence) {
	if(whence == SEEK_SET) {
		return vfs_offset(file, offset);
	} else if(whence == SEEK_CUR) {
		return vfs_offset(file, file->offset + offset);
	} else if(whence == SEEK_END) {
		return vfs_offset(file, vfs_get_size(file) + offset);
	}

	return -1;
}