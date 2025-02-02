#include "../src/behavior_subject.h"
#include "test.h"

static void test_init_behavior_subject() {
    BehaviorSubject subject;
    init_behavior_subject(&subject, "Yikes");

    ASSERT_STR_EQUAL("Yikes", subject.value);
    ASSERT_INT_EQUAL(0, subject.subscriber_count);
    ASSERT_INT_EQUAL(1, subject.capacity);

    unsubscribe(&subject);
}

static void test_notify(const char* value) { (void)value; }
static void test_subscribe() {
    BehaviorSubject subject;
    init_behavior_subject(&subject, "Yikes");
    subscribe(&subject, test_notify);

    ASSERT_STR_EQUAL("Yikes", subject.value);
    ASSERT_INT_EQUAL(1, subject.subscriber_count);
    ASSERT_INT_EQUAL(1, subject.capacity);

    unsubscribe(&subject);
}

static void test_next() {
    BehaviorSubject subject;
    init_behavior_subject(&subject, "Yikes");
    subscribe(&subject, test_notify);

    ASSERT_STR_EQUAL("Yikes", subject.value);
    ASSERT_INT_EQUAL(1, subject.subscriber_count);

    next(&subject, "OK");
    ASSERT_STR_EQUAL("OK", subject.value);
    ASSERT_INT_EQUAL(1, subject.capacity);

    unsubscribe(&subject);
}

static void test_unsubscribe() {
    BehaviorSubject subject;
    init_behavior_subject(&subject, "Yikes");
    subscribe(&subject, test_notify);

    ASSERT_STR_EQUAL("Yikes", subject.value);
    ASSERT_INT_EQUAL(1, subject.subscriber_count);

    next(&subject, "OK");
    ASSERT_STR_EQUAL("OK", subject.value);

    unsubscribe(&subject);

    ASSERT_NULL(subject.subscribers);
    ASSERT_NULL(subject.value);
    ASSERT_INT_EQUAL(0, subject.subscriber_count);
    ASSERT_INT_EQUAL(0, subject.capacity);
}

int main(void) {
    printf("\nTEST BEHAVIOR_SUBJECT\n");

    run_test(test_init_behavior_subject);
    run_test(test_subscribe);
    run_test(test_next);
    run_test(test_unsubscribe);

    return print_test();
}
