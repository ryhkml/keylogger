#include <setjmp.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
//
#include <cmocka.h>

#include "../src/behavior_subject.h"

static void test_init_behavior_subject() {
    BehaviorSubject subject;
    init_behavior_subject(&subject, "Yikes");

    assert_string_equal(subject.value, "Yikes");
    assert_int_equal(subject.subscriber_count, 0);
    assert_int_equal(subject.capacity, 1);

    unsubscribe(&subject);
}

void test_notify(const char* value) { (void)value; }
static void test_subscribe() {
    BehaviorSubject subject;
    init_behavior_subject(&subject, "Yikes");
    subscribe(&subject, test_notify);

    assert_string_equal(subject.value, "Yikes");
    assert_int_equal(subject.subscriber_count, 1);
    assert_int_equal(subject.capacity, 1);

    unsubscribe(&subject);
}

static void test_next() {
    BehaviorSubject subject;
    init_behavior_subject(&subject, "Yikes");
    subscribe(&subject, test_notify);

    assert_string_equal(subject.value, "Yikes");
    assert_int_equal(subject.subscriber_count, 1);

    next(&subject, "OK");
    assert_string_equal(subject.value, "OK");
    assert_int_equal(subject.capacity, 1);

    unsubscribe(&subject);
}

static void test_unsubscribe() {
    BehaviorSubject subject;
    init_behavior_subject(&subject, "Yikes");
    subscribe(&subject, test_notify);

    assert_string_equal(subject.value, "Yikes");
    assert_int_equal(subject.subscriber_count, 1);

    next(&subject, "OK");
    assert_string_equal(subject.value, "OK");

    unsubscribe(&subject);

    assert_null(subject.subscribers);
    assert_null(subject.value);
    assert_int_equal(subject.subscriber_count, 0);
    assert_int_equal(subject.capacity, 0);
}

int main(void) {
    printf("\n");
    printf("TEST BEHAVIOR_SUBJECT\n");
    const struct CMUnitTest tests[] = {cmocka_unit_test(test_init_behavior_subject), cmocka_unit_test(test_subscribe),
                                       cmocka_unit_test(test_next), cmocka_unit_test(test_unsubscribe)};
    return cmocka_run_group_tests(tests, NULL, NULL);
}
