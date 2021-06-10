#include "vfs.h"
#include "debug.h"
#include "string.h"
#include "vga_text.h"

int vfs_lookup(char* path_abs, struct inode* inode) {
    ASSERT(path_abs != NULL);
    PANIC("NOT IMPLEMENTED!");
    struct inode *root_node = vfs_mountpoint(path_tokenize(path_abs));

    char** paths = path_tokenize(path_abs);
    int current_path_index = 0;
    int last_path_index = path_length(paths);

    while(root_node != NULL) {
        struct dentry *dentry_list;
        int dentry_size = root_node->i_op->readdir(root_node, dentry_list);
        dentry_list->inode->refcount++;

        for(int i=0;i<dentry_size;i++) {
            if(!strcmp(paths[current_path_index], dentry_list[i].name)) {
                inode = dentry_list[i].inode;
                current_path_index++;
            } else {
                root_node = NULL;
            }
        }
    }

    if(current_path_index != last_path_index) {
        inode = NULL;
        return -FS_NO_ENTRY;
    }

    return 0;
}