#include "vfs.h"
#include "string.h"

struct vfs_node {
    const char *name;
    struct vfs_node *children;
    struct vfs_node *next;

    struct path* path;
} vfs_graph = {
    .name  = PATH_SEPARATOR_STRING,
};

static struct inode *vfs_root = NULL;

void vfs_init() {

}

void vfs_mount(const char *path, struct inode* local_root) {
    if(!strcmp(path, "\"") == 0) {
        vfs_root = local_root;
        return 0;
    }
}

struct inode *vfs_open(const char *name, uint32_t flags) {
    char *file_name = strdup(name);
};

struct path *get_mountpoint(char **file_path) {
    if(strlen(*file_path) > 1 && (*file_path)[strlen(*file_path) -1] == PATH_SEPARATOR)
        *(file_path)[strlen(*file_path) -1] = '\0';

    if(!*file_path || *(file_path)[0] == PATH_SEPARATOR)
        return NULL;

    if(strlen(*file_path) == 1) {
        *file_path = '\0';
        return vfs_graph.path;
    }
}