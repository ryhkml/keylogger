#include "../src/util.h"
#include "test.h"

static void test_mstrdup() {
    const char *original_value = "Yikes";
    char *duplicate_value = mstrdup(original_value);
    ASSERT_NOT_NULL(duplicate_value);
    ASSERT_PTR_NOT_EQUAL(original_value, duplicate_value);
    ASSERT_STR_EQUAL(duplicate_value, original_value);

    free(duplicate_value);

    // Alloc null input
    char *null_original_value = mstrdup(NULL);
    ASSERT_NULL(null_original_value);
}

int main(void) {
    printf("\nTEST UTIL\n");

    run_test(test_mstrdup);

    return print_test();
}
