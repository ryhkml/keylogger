#include "test.h"

TestSummary test_summary = {
    .run = 0,
    .failed = 0,
    .passed = 0,
};

int current_test_failed = 0;
int counter_assert_run = 0;

void run_test(void (*fn)()) {
    current_test_failed = 0;
    test_summary.run++;

    fn();

    if (current_test_failed == 0) {
        test_summary.passed++;
    } else {
        test_summary.failed++;
    }
}

int print_test() {
    printf("%d PASS\n", counter_assert_run - test_summary.failed);
    printf("%d FAIL\n", test_summary.failed);

    if (test_summary.run == test_summary.passed) {
        return EXIT_SUCCESS;
    }
    return EXIT_FAILURE;
}
