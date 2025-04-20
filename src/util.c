#include "util.h"

#include <stdlib.h>
#include <string.h>

char *mstrdup(const char *value) {
    if (!value) return NULL;
    size_t len = strlen(value);
    void *new_value = malloc(len + 1);
    if (!new_value) return NULL;
    return (char *)memcpy(new_value, value, len + 1);
}
