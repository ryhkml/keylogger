#include <unistd.h>

#include "../src/keylogger.h"
#include "test.h"

static void test_get_key_name() {
    // Word normal
    ASSERT_STR_EQUAL(get_key_name(KEY_A, false, false), "a");
    // Word shifted
    ASSERT_STR_EQUAL(get_key_name(KEY_A, true, false), "A");
    // Word capslocked
    ASSERT_STR_EQUAL(get_key_name(KEY_A, false, true), "A");
    // Word shifted and capslocked
    ASSERT_STR_EQUAL(get_key_name(KEY_A, true, true), "a");

    // Number normal
    ASSERT_STR_EQUAL(get_key_name(KEY_1, false, false), "1");
    // Number shifted
    ASSERT_STR_EQUAL(get_key_name(KEY_1, true, false), "!");
    // Number capslocked
    ASSERT_STR_EQUAL(get_key_name(KEY_1, false, true), "1");
    // Number shifted and capslocked
    ASSERT_STR_EQUAL(get_key_name(KEY_1, true, true), "!");

    // Symbol normal
    ASSERT_STR_EQUAL(get_key_name(KEY_SLASH, false, false), "/");
    // Symbol shifted
    ASSERT_STR_EQUAL(get_key_name(KEY_SLASH, true, false), "?");
    // Symbol capslocked
    ASSERT_STR_EQUAL(get_key_name(KEY_SLASH, false, true), "/");
    // Symbol shifted and capslocked
    ASSERT_STR_EQUAL(get_key_name(KEY_SLASH, true, true), "?");

    // Another key
    ASSERT_STR_EQUAL(get_key_name(-1, false, false), "UNKNOWN");
    ASSERT_STR_EQUAL(get_key_name(9999, false, false), "UNKNOWN");
}

//
// The test function below may cause permission errors because it requires sudo to access /dev/input/event*
// Run the test with sudo or use the rootless method as described in the README.
//
static void test_find_keyboard_device() {
    // Default input
    char *keyboard_path = find_keyboard_device(NULL);
    ASSERT_NOT_NULL(keyboard_path);
    ASSERT_NOT_NULL(strstr(keyboard_path, "/dev/input/event"));

    free(keyboard_path);

    // Custom input
    char *custom_keyboard_path = find_keyboard_device("/dev/input/event1");
    ASSERT_STR_EQUAL(custom_keyboard_path, "/dev/input/event1");

    free(custom_keyboard_path);
}

static void test_notify(const char *key) { (void)key; }
static void test_log_key() {
    FILE *fp = fopen(LOG_FILE, "w");
    ASSERT_NOT_NULL(fp);

    BehaviorSubject subject;
    init_behavior_subject(&subject, "Meta");
    subscribe(&subject, test_notify);

    log_key(fp, &subject, false, false, false, "Meta");
    fclose(fp);
    unsubscribe(&subject);

    // access(LOG_FILE, F_OK)
    ASSERT_INT_EQUAL(access(LOG_FILE, F_OK), 0);
}

int main(void) {
    printf("\nTEST KEYLOGGER\n");

    run_test(test_get_key_name);
    run_test(test_find_keyboard_device);
    run_test(test_log_key);

    return print_test();
}
