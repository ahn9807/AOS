#include "device.h"
#include "ata.h"
#include "keyboard.h"
#include "kmalloc.h"
#include "lib/debug.h"
#include "lib/string.h"
#include "vfs.h"

static int current_bdev_index = 0;
static int current_cdev_index = 0;

static int dev_bwrite(device_t *dev, size_t offset, size_t len, void *buffer);
static int dev_bread(device_t *dev, size_t offset, size_t len, void *buffer);
static inode_t *get_inode();

extern uintptr_t _start_device_probe;
extern uintptr_t _end_device_probe;
typedef void (*__ctor)(void);

static LIST_HEAD(device_list);

void dev_init()
{
	__ctor *start_f = (void *)&_start_device_probe;
	__ctor *end_f = (void *)&_end_device_probe;

	size_t nr = end_f - start_f;

	for (size_t i = 0; i < nr; i++) {
		if (start_f[i]) {
			start_f[i]();
		}
	}
}

void dev_install(device_t *dev, char *path)
{
	ASSERT(dev != NULL);

	char *new_path;
	char num[10];
	inode_t *root_node;

	if (dev->device_type == DEVICE_BLOCK) {
		root_node = get_inode();
		root_node->permission = FS_BLOCKDEVICE;
		itoa(num, 'd', current_bdev_index);
		current_bdev_index++;
		if (path == NULL) {
			new_path = kmalloc(sizeof(DEVICE_BLOCK_DEAFULT_NAME) + sizeof(num) + 1);
			strcpy(new_path, DEVICE_BLOCK_DEAFULT_NAME);
			strcat(new_path, num);
		} else {
			new_path = kmalloc(sizeof(path) + sizeof(num) + 1);
			strcpy(new_path, path);
			strcat(new_path, num);
		}
	} else if (dev->device_type == DEVICE_CHARACTER) {
		root_node = get_inode();
		root_node->permission = FS_CHARDEVICE;
		itoa(num, 'd', current_cdev_index++);
		if (path == NULL) {
			new_path = kmalloc(sizeof(DEVICE_CHAR_DEFAULT_NAME) + sizeof(num) + 1);
			strcpy(new_path, DEVICE_CHAR_DEFAULT_NAME);
			strcat(new_path, num);
		} else {
			new_path = kmalloc(sizeof(path) + sizeof(num) + 1);
			strcpy(new_path, path);
			strcat(new_path, num);
		}
	} else {
		PANIC("DEVICE TYPE ERROR %d", dev->device_type);
	}

	dev->name = new_path;
	root_node->device = dev;

	new_path = kmalloc(strlen(new_path) + strlen(DEVICE_ROOT) + 1);
	strcpy(new_path, DEVICE_ROOT);
	strcat(new_path, dev->name);

	printf("dev path: %s\n", dev->name);
	list_add_tail(&dev->list, &device_list);
	vfs_bind(new_path, root_node);
	kfree(new_path);
}

void dev_uninstall(device_t *dev)
{
	PANIC("NOT IMPLEMENTED");
}

/* Reads SIZE bytes from INODE into BUFFER, starting at position OFFSET.
 * Returns the number of bytes actually read, which may be less
 * than SIZE if an error occurs or end of file is reached. */
size_t dev_read(device_t *dev, size_t offset, size_t size, void *buf)
{
	ASSERT(dev != NULL);
	ASSERT(buf != NULL);
	if (dev->device_type == DEVICE_BLOCK) {
		ASSERT(dev->dev_op->read != NULL);
		return dev_bread(dev, offset, size, buf);
	} else if (dev->device_type == DEVICE_CHARACTER) {
		return dev->dev_op->read(dev->aux, offset, size, buf);
	} else {
		PANIC("DEVICE TYPE ERROR %d", dev->device_type);
	}

	return 0;
}

/* Writes SIZE bytes from INODE into BUFFER, starting at position OFFSET.
 * Returns the number of bytes actually write, which may be less
 * than SIZE if an error occurs or end of file is reached. */
size_t dev_write(device_t *dev, size_t offset, size_t size, void *buf)
{
	ASSERT(dev != NULL);
	ASSERT(buf != NULL);
	if (dev->device_type == DEVICE_BLOCK) {
		ASSERT(dev->dev_op->write != NULL);
		return dev_bwrite(dev, offset, size, buf);
	} else if (dev->device_type == DEVICE_CHARACTER) {
		return dev->dev_op->write(dev->aux, offset, size, buf);
	} else {
		PANIC("DEVICE TYPE ERROR %d", dev->device_type);
	}

	return 0;
}

static inode_t *get_inode()
{
	inode_t *inode = kcalloc(1, sizeof(inode_t));
	inode->atime = 0;
	inode->mtime = 0;
	inode->ctime = 0;
}

static int dev_bread(device_t *dev, size_t offset, size_t len, void *buffer)
{
	uint32_t block_size = dev->dev_op->block_size(dev->aux);
	uint32_t start_block = offset / block_size;
	uint32_t end_block = (offset + len - 1) / block_size;

	uint64_t x_offset = 0;

	uint8_t *buffer_8 = (uint8_t *)buffer;

	char tmp[block_size];

	if (offset % block_size) {
		uint32_t prefix_size = (block_size - (offset % block_size));
		dev->dev_op->read(dev->aux, start_block * block_size, block_size, tmp);
		memcpy(buffer_8, (void *)((uintptr_t)tmp + ((uintptr_t)offset % block_size)), prefix_size);

		x_offset += prefix_size;
		start_block++;
	}
	if ((offset + len) % block_size && start_block < end_block) {
		uint32_t postfix_size = (offset + len) % block_size;
		dev->dev_op->read(dev->aux, end_block * block_size, block_size, tmp);
		memcpy((void *)((uintptr_t)buffer_8 + len - postfix_size), tmp, postfix_size);

		end_block--;
	}

	while (start_block <= end_block) {
		dev->dev_op->read(dev->aux, start_block * block_size, block_size,
				  (void *)((uintptr_t)buffer_8 + x_offset));
		x_offset += block_size;
		start_block++;
	}

	return len;
}

static int dev_bwrite(device_t *dev, size_t offset, size_t len, void *buffer)
{
	uint32_t block_size = dev->dev_op->block_size(dev->aux);
	uint32_t start_block = offset / block_size;
	uint32_t end_block = (offset + len - 1) / block_size;

	uint64_t x_offset = 0;

	uint8_t *buffer_8 = (uint8_t *)buffer;

	char tmp[block_size];

	if (offset % block_size) {
		dev->dev_op->read(dev->aux, start_block * block_size, block_size, tmp);
		uint32_t prefix_size = (block_size - (offset % block_size));
		memcpy((void *)((uintptr_t)tmp + ((uintptr_t)offset % block_size)), buffer_8, prefix_size);
		dev->dev_op->write(dev->aux, start_block * block_size, block_size, tmp);

		x_offset += prefix_size;
		start_block++;
	}
	if ((offset + len) % block_size && start_block <= end_block) {
		uint32_t postfix_size = (offset + len) % block_size;
		dev->dev_op->read(dev->aux, end_block * block_size, block_size, tmp);
		memcpy(tmp, (void *)((uintptr_t)buffer_8 + len - postfix_size), postfix_size);
		dev->dev_op->write(dev->aux, end_block * block_size, block_size, tmp);

		end_block--;
	}

	while ((int)start_block <= (int)end_block) {
		dev->dev_op->write(dev->aux, start_block * block_size, block_size,
				   (void *)((uintptr_t)buffer_8 + x_offset));
		x_offset += block_size;
		start_block++;
	}

	return len;
}