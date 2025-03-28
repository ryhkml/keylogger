#include "util.h"

#include <stdlib.h>
#include <string.h>

char *mstrdup(const char *value) {
    if (!value) return NULL;

    size_t len = strlen(value) + 1;
    char *new_value = malloc(len);

    if (new_value) memcpy(new_value, value, len);
    return new_value;
}
