#include <linux/input-event-codes.h>
#include <setjmp.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//
#include <cmocka.h>
#include <unistd.h>

#include "../src/keylogger.h"

static void test_get_key_name() {
    // Word normal
    assert_string_equal(get_key_name(KEY_A, false, false), "a");
    // Word shifted
    assert_string_equal(get_key_name(KEY_A, true, false), "A");
    // Word capslocked
    assert_string_equal(get_key_name(KEY_A, false, true), "A");
    // Word shifted and capslocked
    assert_string_equal(get_key_name(KEY_A, true, true), "a");

    // Number normal
    assert_string_equal(get_key_name(KEY_1, false, false), "1");
    // Number shifted
    assert_string_equal(get_key_name(KEY_1, true, false), "!");
    // Number capslocked
    assert_string_equal(get_key_name(KEY_1, false, true), "1");
    // Number shifted and capslocked
    assert_string_equal(get_key_name(KEY_1, true, true), "!");

    // Symbol normal
    assert_string_equal(get_key_name(KEY_SLASH, false, false), "/");
    // Symbol shifted
    assert_string_equal(get_key_name(KEY_SLASH, true, false), "?");
    // Symbol capslocked
    assert_string_equal(get_key_name(KEY_SLASH, false, true), "/");
    // Symbol shifted and capslocked
    assert_string_equal(get_key_name(KEY_SLASH, true, true), "?");

    // Another key
    assert_string_equal(get_key_name(-1, false, false), "UNKNOWN");
    assert_string_equal(get_key_name(9999, false, false), "UNKNOWN");
}

//
// The test function below may cause permission errors because it requires sudo to access /dev/input/event*
// Run the test with sudo or use the rootless method as described in the README.
//
static void test_find_keyboard_device() {
    // Default input
    char *keyboard_path = find_keyboard_device(NULL);
    assert_non_null(keyboard_path);
    assert_non_null(strstr(keyboard_path, "/dev/input/event"));

    free(keyboard_path);

    // Custom input
    char *custom_keyboard_path = find_keyboard_device("/dev/input/event1");
    assert_string_equal(custom_keyboard_path, "/dev/input/event1");

    free(custom_keyboard_path);
}

void test_notify(const char *key) { (void)key; }
static void test_log_key() {
    FILE *fp = fopen(LOG_FILE, "w");
    assert_non_null(fp);

    BehaviorSubject subject;
    init_behavior_subject(&subject, "Meta");
    subscribe(&subject, test_notify);

    log_key(fp, &subject, false, false, false, "Meta");
    fclose(fp);
    unsubscribe(&subject);

    assert_int_equal(access(LOG_FILE, F_OK), 0);
}

int main(void) {
    printf("\n");
    printf("TEST KEYLOGGER\n");
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_get_key_name), cmocka_unit_test(test_find_keyboard_device),
                                       cmocka_unit_test(test_log_key)};
    return cmocka_run_group_tests(tests, NULL, NULL);
}
