#include "vfs.h"
#include "string.h"
#include "kmalloc.h"
#include "vga_text.h"
#include "debug.h"
#include "list.h"

// This is temp directory tree for file system
// Have to change this
struct vfs_node vfs_tree = {
    .name  = PATH_SEPARATOR_STRING,
    .fs = NULL,
};

struct inode *vfs_root = NULL;
struct list file_systems;

// Initialize vfs system
void vfs_init() {
    list_init(&file_systems);
}

// Get mountpoint of the path
// Mountpoint must be initialized with dev_install
// Without this it will return / always
struct vfs_node *vfs_mountpoint(char *path) {
    char **file_path = path_tokenize(path);
    if(strlen(*file_path) > 1 && (*file_path)[strlen(*file_path) -1] == PATH_SEPARATOR)
        *(file_path)[strlen(*file_path) -1] = '\0';

    if(!*file_path || *(file_path)[0] == PATH_SEPARATOR)
        return NULL;

    struct vfs_node *cur_node = &vfs_tree;
    struct vfs_node *last_target_node = cur_node;

    size_t token_index = 0;
    int check_last_node = 0;

    for(char **token_p = file_path; *token_p; ++token_p) {
        char *token = *token_p;

        check_last_node = 0;

        if(cur_node->inode != NULL) {
            last_target_node = cur_node;
        }
        if (cur_node->children != NULL) {
            cur_node = cur_node->children;
            for (struct vfs_node *m_node = cur_node; m_node; m_node = m_node->next) {
                if (!strcmp(token, m_node->name)) {
                    cur_node = m_node;
                    check_last_node = 1;
                    goto next;
                }
            }
            break;
        } else {
            break;
        }
next:;
        ++token_index;
    }

    if (check_last_node && cur_node->inode) {
        last_target_node = cur_node;
    }

    return last_target_node;
}

// Bind mountpoint to the vfs tree structure
// Install Device
int vfs_bind(const char *path, struct inode *target)
{
    if (!path ||  !*path || !target)
        return -FS_INVALID;

    if(!strcmp(path, PATH_SEPARATOR_STRING)) {
        vfs_root = target;
        vfs_tree.inode = target;
        vfs_tree.children = NULL;
    
        return 0;
    }

    char **tokens = path_tokenize(path);

    struct vfs_node *cur_node = &vfs_tree;

    for (char **token_p = tokens; *token_p; ++token_p) {
        char *token = *token_p;
        if (cur_node->children != NULL) {
            cur_node = cur_node->children;

            struct vfs_node *last_node = NULL;
            for (struct vfs_node *node = cur_node; node; node = node->next) {
                last_node = node;
                if (!strcmp(node->name, token)) {
                    cur_node = node;
                    goto next;
                }
            }

            struct vfs_node *new_node = kcalloc(sizeof(struct vfs_node), 1);
            ASSERT(new_node != NULL);

            new_node->name = strdup(token);
            new_node->children = NULL;
            new_node->fs = NULL;
            last_node->next = new_node;
            cur_node = new_node;
        } else {
            struct vfs_node *new_node = kcalloc(sizeof(struct vfs_node), 1);
            ASSERT(new_node != NULL);

            new_node->name = strdup(token);
            new_node->children = NULL;
            new_node->fs = NULL;
            cur_node->children = new_node;
            cur_node = new_node;
        }
next:;
    }

    cur_node->full_path = strdup(path);
    cur_node->inode = target;
    return 0;
}

// Install filesystem
int vfs_install(struct vfs_fs *fs) {
    list_push_back(&file_systems, &fs->elem);
}

// Get filesystem
struct vfs_fs *vfs_find(char *name) {
    for(struct list_elem *e = list_begin(&file_systems); e != list_end(&file_systems); e = list_next(&file_systems)) {
        if(!strcmp(list_entry(e, struct vfs_fs, elem)->name, name)) {
            return list_entry(e, struct vfs_fs, elem);
        }
    }

    return NULL;
}