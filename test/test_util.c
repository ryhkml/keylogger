#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <cmocka.h>

#include "../src/util.h"

static void test_mstrdup() {
    const char *original_value = "Yikes";

    char *duplicate_value = mstrdup(original_value);
    assert_non_null(duplicate_value);
    assert_ptr_not_equal(duplicate_value, original_value);
    assert_string_equal(original_value, duplicate_value);

    free(duplicate_value);

    // Alloc null input
    char *null_original_value = mstrdup(NULL);
    assert_null(null_original_value);
}

int main(void) {
    printf("\n");
    printf("TEST UTIL\n");
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_mstrdup)};
    return cmocka_run_group_tests(tests, NULL, NULL);
}
