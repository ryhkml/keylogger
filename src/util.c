#include "util.h"

#include <stdlib.h>
#include <string.h>

char *mstrdup(const char *value) {
    if (!value) return NULL;
    size_t len = strlen(value);
    char *new_value = malloc(len + 1);
    if (new_value) strncpy(new_value, value, len + 1);
    return new_value;
}
