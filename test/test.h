#ifndef TEST_CONFIG_H
#define TEST_CONFIG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define COLOR_RED "\033[0;31m"
#define COLOR_RESET "\033[0m"

typedef struct {
    int run;
    int failed;
    int passed;
} TestSummary;

extern TestSummary test_summary;

extern int current_test_failed;
extern int counter_assert_run;

void run_test(void (*fn)());
int print_test();

#define ASSERT_STR_EQUAL(expected, actual)                                                                        \
    do {                                                                                                          \
        counter_assert_run++;                                                                                     \
        if (strcmp((expected), (actual)) != 0) {                                                                  \
            fprintf(stderr, "%s[FAIL]%s Expected: %s, Actual: %s -> %s:%d\n", COLOR_RED, COLOR_RESET, (expected), \
                    (actual), __FILE__, __LINE__);                                                                \
            current_test_failed = 1;                                                                              \
        } else {                                                                                                  \
            printf("[PASS] -> %s\n", __FILE__);                                                                   \
        }                                                                                                         \
    } while (0)

#define ASSERT_INT_EQUAL(expected, actual)                                                                \
    do {                                                                                                  \
        counter_assert_run++;                                                                             \
        if ((expected) != (actual)) {                                                                     \
            fprintf(stderr, "%s[FAIL]%s Expected: %lld, Actual: %lld -> %s:%d\n", COLOR_RED, COLOR_RESET, \
                    (long long)(expected), (long long)(actual), __FILE__, __LINE__);                      \
            current_test_failed = 1;                                                                      \
        } else {                                                                                          \
            printf("[PASS] -> %s\n", __FILE__);                                                           \
        }                                                                                                 \
    } while (0)

#define ASSERT_NULL(actual)                                                                                      \
    do {                                                                                                         \
        counter_assert_run++;                                                                                    \
        if ((actual) != NULL) {                                                                                  \
            fprintf(stderr, "%s[FAIL]%s Expected: NULL -> %s:%d\n", COLOR_RED, COLOR_RESET, __FILE__, __LINE__); \
            current_test_failed = 1;                                                                             \
        } else {                                                                                                 \
            printf("[PASS] -> %s\n", __FILE__);                                                                  \
        }                                                                                                        \
    } while (0)

#define ASSERT_NOT_NULL(actual)                                                                                      \
    do {                                                                                                             \
        counter_assert_run++;                                                                                        \
        if ((actual) == NULL) {                                                                                      \
            fprintf(stderr, "%s[FAIL]%s Expected: NOT NULL -> %s:%d\n", COLOR_RED, COLOR_RESET, __FILE__, __LINE__); \
            current_test_failed = 1;                                                                                 \
        } else {                                                                                                     \
            printf("[PASS] -> %s\n", __FILE__);                                                                      \
        }                                                                                                            \
    } while (0)

#define ASSERT_PTR_EQUAL(expected, actual)                                                            \
    do {                                                                                              \
        counter_assert_run++;                                                                         \
        if ((expected) != (actual)) {                                                                 \
            fprintf(stderr, "%s[FAIL]%s Expected: %p, Actual: %p -> %s:%d\n", COLOR_RED, COLOR_RESET, \
                    (void *)(expected), (void *)(actual), __FILE__, __LINE__);                        \
            current_test_failed = 1;                                                                  \
        } else {                                                                                      \
            printf("[PASS] -> %s\n", __FILE__);                                                       \
        }                                                                                             \
    } while (0)

#define ASSERT_PTR_NOT_EQUAL(expected, actual)                                                        \
    do {                                                                                              \
        counter_assert_run++;                                                                         \
        if ((expected) == (actual)) {                                                                 \
            fprintf(stderr, "%s[FAIL]%s Expected: %p, Actual: %p -> %s:%d\n", COLOR_RED, COLOR_RESET, \
                    (void *)(expected), (void *)(actual), __FILE__, __LINE__);                        \
            current_test_failed = 1;                                                                  \
        } else {                                                                                      \
            printf("[PASS] -> %s\n", __FILE__);                                                       \
        }                                                                                             \
    } while (0)

#endif  // TEST_CONFIG_H
