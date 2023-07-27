#include "kmalloc.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "vfs.h"
#include "vga_text.h"

// Mount filesystem at super_node from path
int vfs_mount(const char *path, struct inode *super_node)
{
	int ret = 0;
	ret = vfs_bind(path, super_node);

	if (ret) {
		return ret;
	}

	struct vfs_node *local_fs = vfs_mountpoint(path);

	if (local_fs == NULL) {
		return -FS_NO_ENTRY;
	}

	local_fs->fs = vfs_find("ext2");
	local_fs->fs->fs_op->mount(local_fs->inode->device, super_node, local_fs->aux);
	local_fs->inode = super_node;

	return 0;
}