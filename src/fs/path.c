#include "vfs.h"
#include "kmalloc.h"
#include "string.h"
#include "queue.h"

// Check there exist element s in the path
int path_search(const char *path, const char *s) {
    if(path == NULL || s == NULL) {
        return 0;
    }
    for(int i=0;;i++) {
        if(path[i] == NULL) {
            if(!strcmp(path[i], s)) {
                return 1;
            }
        }
    }

    return 0;
}

// parse path to the token
// eg. /a/b/c/.. -> a b c ..
char **path_tokenize(const char *path) {
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

// get length of char**
int path_length(char **path) {
    int size = 0;
    for(int i=0;path[i] != NULL; i++) {
        size++;
    }
    return size;
}

// This method parse abosolute path from cwd and path
// eg. /a/b/c/ ../../d = /a/d
char *path_absolute(char *cwd, char *path) {
    queue_t *stack = queue_init();
    char **_cwd = path_tokenize(cwd);
    char **_path = path_tokenize(path);

    int parsed_str_len = 0;
    int current_cwd_index = path_length(_cwd) - 1;

    for(int i=0;i<=current_cwd_index;i++) {
        queue_insert_tail(stack, _cwd[i]);
        parsed_str_len += strlen(_cwd[i]);
    }

    if(strlen(path) && path[0] != PATH_SEPARATOR) {
        for(int i=0; _path[i] != NULL; i++) {
            if(_path[i] == PATH_CUR) {
                // . -> do nothing
            } else if(!strcmp(_path[i], PATH_CHILD) && !queue_is_empty(stack)) {
                // .. -> pop tail
                parsed_str_len -= strlen(queue_get_tail(stack));
                current_cwd_index -= 1;
            } else {
                // else -> push to the stack
                queue_insert_tail(stack, _path[i]);
                parsed_str_len += strlen(_path[i]);
                current_cwd_index += 1;
            }
        }
    }

    char *parsed_str = kmalloc(parsed_str_len + current_cwd_index + 1);
    int str_index = 0;

    if(queue_is_empty(stack)) {
        strcpy(parsed_str, PATH_SEPARATOR_STRING);
    }

    while(!queue_is_empty(stack)) {
        strcpy(&parsed_str[str_index++], PATH_SEPARATOR_STRING);
        char* temp_str = queue_get_head(stack);
        strcpy(&parsed_str[str_index], temp_str);
        str_index += strlen(temp_str);
    }
    queue_free(stack);

    return parsed_str;
}