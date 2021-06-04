#include "vfs.h"
#include "string.h"
#include "kmalloc.h"
#include "vga_text.h"
#include "debug.h"

// This is temp directory tree for file system
// Have to change this
struct vfs_node {
    char *name;
    struct vfs_node *children;
    struct vfs_node *next;

    struct inode *inode;
} vfs_graph = {
    .name  = PATH_SEPARATOR_STRING,
    .inode = NULL,
};

struct inode *vfs_root = NULL;

void vfs_init() {

}

struct path *vfs_mountpoint(char **file_path) {
    if(strlen(*file_path) > 1 && (*file_path)[strlen(*file_path) -1] == PATH_SEPARATOR)
        *(file_path)[strlen(*file_path) -1] = '\0';

    if(!*file_path || *(file_path)[0] == PATH_SEPARATOR)
        return NULL;

    struct path* path = kmalloc(sizeof(struct path));
    path->tokens = file_path;

    struct vfs_node *cur_node = &vfs_graph;
    struct vfs_node *last_target_node = cur_node;

    size_t token_index = 0;
    int check_last_node = 0;

    for(char **token_p = file_path; *token_p; ++token_p) {
        char *token = *token_p;

        check_last_node = 0;

        if(cur_node->inode != NULL) {
            last_target_node = cur_node;
            path->tokens = file_path + token_index;
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
        path->tokens = file_path + token_index;
    }

    path->root = last_target_node->inode;

    return path;
}

int vfs_bind(const char *path, struct inode *target)
{
    if (!path ||  !*path || !target)
        return -FS_INVALID;

    if (!strcmp(path, PATH_SEPARATOR_STRING)) {
        ASSERT("YOU CANNOT MOUNT AT ROOT!");
        return 0;
    }

    char **tokens = path_tokenize(path);

    struct vfs_node *cur_node = &vfs_graph;

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
            new_node->inode = NULL;
            last_node->next = new_node;
            cur_node = new_node;
        } else {
            struct vfs_node *new_node = kcalloc(sizeof(struct vfs_node), 1);
            ASSERT(new_node != NULL);

            new_node->name = strdup(token);
            new_node->children = NULL;
            new_node->inode = NULL;
            cur_node->children = new_node;
            cur_node = new_node;
        }
next:;
    }

    cur_node->inode = target;
    return 0;
}