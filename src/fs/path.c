#include "vfs.h"
#include "kmalloc.h"
#include "string.h"

void get_path(struct path *_path, struct inode *root, const char *path) {
    if(path == NULL || *path == '\0')
        return NULL;

    while (*path == PATH_SEPARATOR)
        ++path;

    char *tokens = strdup(path);

    if (tokens == NULL)
        return NULL;

    int len = strlen(path);

    if (!len) {
        char **ret = kmalloc(sizeof(char *));
        *ret = NULL;
        return ret;
    }

    int i, count = 0;
    for (i = 0; i < len; ++i) {
        if (tokens[i] == PATH_SEPARATOR) {
            tokens[i] = 0;
            ++count;
        }
    }

    if (path[len-1] != PATH_SEPARATOR)
        ++count;

    char **ret = kmalloc(sizeof(char *) * (count + 1));

    int j = 0;
    ret[j++] = tokens;

    for (i = 0; i < strlen(path) - 1; ++i)
        if (tokens[i] == 0)
            ret[j++] = &tokens[i+1];

    ret[j] = NULL;

    _path->tokens = ret;
    _path->root = root;
}

int search_path(const struct path *path, const char *s) {
    if(path == NULL || s == NULL) {
        return 0;
    }
    for(int i=0;;i++) {
        if(path->tokens[i] == NULL) {
            if(!strcmp(path->tokens[i], s)) {
                return 1;
            }
        }
    }

    return 0;
}

char **tokenize(const char *path) {
       if(path == NULL || *path == '\0')
        return NULL;

    while (*path == PATH_SEPARATOR)
        ++path;

    char *tokens = strdup(path);

    if (tokens == NULL)
        return NULL;

    int len = strlen(path);

    if (!len) {
        char **ret = kmalloc(sizeof(char *));
        *ret = NULL;
        return ret;
    }

    int i, count = 0;
    for (i = 0; i < len; ++i) {
        if (tokens[i] == PATH_SEPARATOR) {
            tokens[i] = 0;
            ++count;
        }
    }

    if (path[len-1] != PATH_SEPARATOR)
        ++count;

    char **ret = kmalloc(sizeof(char *) * (count + 1));

    int j = 0;
    ret[j++] = tokens;

    for (i = 0; i < strlen(path) - 1; ++i)
        if (tokens[i] == 0)
            ret[j++] = &tokens[i+1];

    ret[j] = NULL;

    return ret;
}