#include "vfs.h"

size_t vfs_offset(struct file *file, size_t offset) {
	if(file == NULL) {
		return -FS_INVALID;
	}

	file->offset = offset;
}