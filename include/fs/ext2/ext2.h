#pragma once

#include <stddef.h>
#include <stdint.h>
#include "vfs.h"

/* File metadata */
#define EXT2_MAGIC 0xEF53
#define EXT2_DIRECT_BLOCKS 12
#define EXT2_DATE		"95/08/09"
#define EXT2_VERSION		"0.5b"

/* File Types */
#define EXT2_S_IFSOCK	0xC000
#define EXT2_S_IFLNK	0xA000
#define EXT2_S_IFREG	0x8000
#define EXT2_S_IFBLK	0x6000
#define EXT2_S_IFDIR	0x4000
#define EXT2_S_IFCHR	0x2000
#define EXT2_S_IFIFO	0x1000

/* setuid, etc. */
#define EXT2_S_ISUID	0x0800
#define EXT2_S_ISGID	0x0400
#define EXT2_S_ISVTX	0x0200

/* rights */
#define EXT2_S_IRUSR	0x0100
#define EXT2_S_IWUSR	0x0080
#define EXT2_S_IXUSR	0x0040
#define EXT2_S_IRGRP	0x0020
#define EXT2_S_IWGRP	0x0010
#define EXT2_S_IXGRP	0x0008
#define EXT2_S_IROTH	0x0004
#define EXT2_S_IWOTH	0x0002
#define EXT2_S_IXOTH	0x0001

/* File system states */
#define	EXT2_VALID_FS			0x0001	/* Unmounted cleanly */
#define	EXT2_ERROR_FS			0x0002	/* Errors detected */
#define	EFSCORRUPTED			EUCLEAN	/* Filesystem is corrupted */

/* Mount flags */
#define EXT2_MOUNT_OLDALLOC		0x000002  /* Don't use the new Orlov allocator */
#define EXT2_MOUNT_GRPID		0x000004  /* Create files with directory's group */
#define EXT2_MOUNT_DEBUG		0x000008  /* Some debugging messages */
#define EXT2_MOUNT_ERRORS_CONT		0x000010  /* Continue on errors */
#define EXT2_MOUNT_ERRORS_RO		0x000020  /* Remount fs ro on errors */
#define EXT2_MOUNT_ERRORS_PANIC		0x000040  /* Panic on errors */
#define EXT2_MOUNT_MINIX_DF		0x000080  /* Mimics the Minix statfs */
#define EXT2_MOUNT_NOBH			0x000100  /* No buffer_heads */
#define EXT2_MOUNT_NO_UID32		0x000200  /* Disable 32-bit UIDs */
#define EXT2_MOUNT_XATTR_USER		0x004000  /* Extended user attributes */
#define EXT2_MOUNT_POSIX_ACL		0x008000  /* POSIX Access Control Lists */
#define EXT2_MOUNT_XIP			0x010000  /* Obsolete, use DAX */
#define EXT2_MOUNT_USRQUOTA		0x020000  /* user quota */
#define EXT2_MOUNT_GRPQUOTA		0x040000  /* group quota */
#define EXT2_MOUNT_RESERVATION		0x080000  /* Preallocation */
#define EXT2_MOUNT_DAX			0x100000  /* Direct Access */

/* Behaviour when detecting errors */
#define EXT2_ERRORS_CONTINUE		1	/* Continue execution */
#define EXT2_ERRORS_RO			2	/* Remount fs read-only */
#define EXT2_ERRORS_PANIC		3	/* Panic */
#define EXT2_ERRORS_DEFAULT		EXT2_ERRORS_CONTINUE

/* Special inode numbers */
#define	EXT2_BAD_INO		 1	/* Bad blocks inode */
#define EXT2_ROOT_INO		 2	/* Root inode */
#define EXT2_BOOT_LOADER_INO	 5	/* Boot loader inode */
#define EXT2_UNDEL_DIR_INO	 6	/* Undelete directory inode */

#define SINGLE_INDIRECT_POINTER(inode) (inode->block[EXT2_DIRECT_BLOCKS + 1])
#define DOUBLE_INDIRECT_POINTER(inode) (inode->block[EXT2_DIRECT_BLOCKS + 2])
#define TRIPLE_INDIRECT_POINTER(inode) (inode->block[EXT2_DIRECT_BLOCKS + 3])


/* Super block struct. */
typedef struct ext2_superblock
{
	uint32_t inodes_count; /* Inodes count */
	uint32_t blocks_count; /* Blocks count */
	uint32_t r_blocks_count; /* Reserved blocks count */
	uint32_t free_blocks_count; /* Free blocks count */
	uint32_t free_inodes_count; /* Free inodes count */
	uint32_t first_data_block; /* First Data Block */
	uint32_t log_block_size; /* Block size */
	uint32_t log_frag_size; /* Fragment size */
	uint32_t blocks_per_group; /* # Blocks per group */
	uint32_t frags_per_group; /* # Fragments per group */
	uint32_t inodes_per_group; /* # Inodes per group */
	uint32_t mtime; /* Mount time */
	uint32_t wtime; /* Write time */

	uint16_t mnt_count; /* Mount count */
	uint16_t max_mnt_count; /* Maximal mount count */
	uint16_t magic; /* Magic signature */
	uint16_t state; /* File system state */
	uint16_t errors; /* Behaviour when detecting errors */
	uint16_t minor_rev_level; /* minor revision level */

	uint32_t lastcheck; /* time of last check */
	uint32_t checkinterval; /* max. time between checks */
	uint32_t creator_os; /* OS */
	uint32_t rev_level; /* Prevision level */

	uint16_t def_resuid; /* Default uid for reserved blocks */
	uint16_t def_resgid; /* Default gid for reserved blocks */

	/*
	 * These fields are for EXT2_DYNAMIC_REV superblocks only.
	 *
	 * Note: the difference between the compatible feature set and
	 * the incompatible feature set is that if there is a bit set
	 * in the incompatible feature set that the kernel doesn't
	 * know about, it should refuse to mount the filesystem.
	 *
	 * e2fsck's requirements are more strict; if it doesn't know
	 * about a feature in either the compatible or incompatible
	 * feature set, it must abort and not try to meddle with
	 * things it doesn't understand...
	 */
	uint32_t first_ino; /* First non-reserved inode */
	uint16_t inode_size; /* size of inode structure */
	uint16_t block_group_nr; /* block group # of this superblock */
	uint32_t feature_compat; /* compatible feature set */
	uint32_t feature_incompat; /* incompatible feature set */
	uint32_t feature_ro_compat; /* readonly-compatible feature set */

	uint8_t uuid[16]; /* 128-bit uuid for volume */
	uint8_t volume_name[16]; /* volume name */

	uint8_t last_mounted[64]; /* directory where last mounted */

	uint32_t algo_bitmap; /* For compression */
	/*
	 * Performance hints.  Directory preallocation should only
	 * happen if the EXT2_COMPAT_PREALLOC flag is on.
	 */
	uint8_t prealloc_blocks; /* Nr of blocks to try to preallocate*/ 
	uint8_t prealloc_dir_blocks; /* Nr to preallocate for dirs */
	uint16_t _padding;
	/*
	 * Journaling support valid if EXT3_FEATURE_COMPAT_HAS_JOURNAL set.
	 */
	uint8_t journal_uuid[16]; /* uuid of journal superblock */
	uint32_t journal_inum; /* inode number of journal file */
	uint32_t jounral_dev; /* device number of journal file */
	uint32_t last_orphan; /* start of list of inodes to delete */

	/* Directory Indexing Support */
	uint32_t hash_seed[4]; /* HTREE hash seed */
	uint8_t def_hash_version; /* Default hash version to use */
	uint16_t _padding_a;
	uint8_t _padding_b;

	/* Other Options */
	uint32_t default_mount_options;
	uint32_t first_meta_bg; /* First metablock block group */
	uint8_t _unused[760]; /* Padding to the end of the block */

} __attribute__((packed)) ext2_superblock_t;

/* Block group descriptor on the disk */
typedef struct ext2_group_desc
{
	uint32_t block_bitmap; /* Blocks bitmap block */
	uint32_t inode_bitmap; /* Inodes bitmap block */
	uint32_t inode_table; /* Inodes table block */
	uint16_t free_blocks_count; /* Free blocks count */
	uint16_t free_inodes_count; /* Free inodes count */
	uint16_t used_dirs_count; /* Directories count */
	uint16_t pad;
	uint8_t reserved[12];
} __attribute__((packed)) ext2_group_desc_t;

/* Inode on the disk */
typedef struct ext2_inode
{
	uint16_t mode; /* File mode (Type and Permissions) */
	uint16_t uid; /* Low 16bits of Owner Uid */
	uint32_t size;  /*file length in byte. */
	uint32_t atime; /* Access time */
	uint32_t ctime; /* Creation time */
	uint32_t mtime; /* Modification time */
	uint32_t dtime; /* Deletion Time */
	uint16_t gid; /* Low 16bits of Group ID */
	uint16_t links_count; /* Links count */
	uint32_t blocks; /* Blocks count */
	uint32_t flags; /* File flags */
	uint32_t osd1; /* OS dependent 1 */
	uint32_t block[15]; /* Pointers to blocks */
	uint32_t generation; /* File version (used by NFS) */
	uint32_t file_acl; /* File ACL (acess control lists) */
	uint32_t dir_acl; /* Directory ACL */
	uint32_t faddr; /* Fragment address */
	uint8_t osd2[12]; /* OS dependent 2 */
} __attribute__((packed)) ext2_inode_t;

typedef struct ext2_dentry
{
	uint32_t inode_nr; /* inode number */
	uint16_t rec_len; /* Directory entry length */
	uint8_t name_len; /* Name length */
	uint8_t file_type; /* Type of file */
	char name[]; /* Actually a set of characters, at most 255 bytes */
} __attribute__((packed)) ext2_dentry_t;

typedef struct ext2_disk_cache_entry
{
	uint32_t block_no;
	uint32_t last_use;
	uint8_t dirty;
	uint8_t *block;
} ext2_disk_cache_entry_t;

typedef struct ext2_fs {
	ext2_superblock_t *superblock;
	ext2_group_desc_t *group_descs;
	uint32_t desc_len;
	uint32_t block_size; // (1024 << desc->log_block_size)
	struct inode *mountpoint;
} ext2_fs_t;

typedef size_t inode_number_t;

int ext2_init();