#include <stdbool.h>
#include "vfs.h"
#include "string.h"
#include "stat.h"
#include "kmalloc.h"
#include "vga_text.h"

int vfs_lookup(inode_t *root_node, char *path, struct dentry *ret_dir)
{
	if (root_node == NULL)
	{
		return -FS_NO_ENTRY;
	}

	bool found = false;
	struct inode *cur_node = root_node;

	char **tokenized_path = path_tokenize(path);

	for (int i = 0; i < path_length(tokenized_path); i++)
	{
		int d_idx = 0;
		found = false;

		while (vfs_readdir(cur_node, d_idx, ret_dir) > 0)
		{
			if (!strcmp(ret_dir->name, tokenized_path[i]))
			{
				cur_node = ret_dir->inode;
				if (ret_dir->inode != NULL)
					found = true;
				break;
			}
			d_idx++;
		}

		if (found == false)
		{
			ret_dir = NULL;
			return -FS_NO_ENTRY;
		}
	}

	if (S_ISDIR(cur_node->type))
	{
		return FS_DIRECTORY;
	}
	else if (S_ISREG(cur_node->type))
	{
		return FS_FILE;
	}
	else
	{
		return FS_NOT_FILE;
	}
}