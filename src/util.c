#include "util.h"

#include <stdlib.h>
#include <string.h>

char *mstrdup(const char *value) {
    if (value == NULL) {
        return NULL;
    }

    size_t len = strlen(value) + 1;
    char *new_value = (char *)malloc(len);

    if (new_value == NULL) {
        return NULL;
    }

    memcpy(new_value, value, len);
    return new_value;
}
