#include "behavior_subject.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define INITIAL_CAPACITY 1
#define MAX_SUBSCRIBERS 1

void init_behavior_subject(BehaviorSubject *subject, const char *initial_value) {
    subject->capacity = MAX_SUBSCRIBERS;
    subject->subscribers = malloc(subject->capacity * sizeof(subscriber_cb));
    if (!subject->subscribers) {
        printf("Failed to allocate memory for subscribers\n");
        subject->subscribers = NULL;
        subject->capacity = 0;
        return;
    }
    subject->subscriber_count = 0;
    if (!initial_value) {
        subject->value = NULL;
    } else {
        subject->value = mstrdup(initial_value);
        if (!subject->value) {
            printf("Failed to allocate memory for initial value\n");
            free(subject->subscribers);
            subject->subscribers = NULL;
            subject->capacity = 0;
        }
    }
}

void subscribe(BehaviorSubject *subject, subscriber_cb callback) {
    if (!subject->subscribers) {
        printf("Subject not initialized\n");
        return;
    }
    if (subject->subscriber_count >= MAX_SUBSCRIBERS) {
        subject->subscribers[0] = callback;
    } else {
        subject->subscribers[subject->subscriber_count] = callback;
        subject->subscriber_count++;
    }
    // Immediately emit the current value to the new subscriber
    if (callback) callback(subject->value);
}

void next(BehaviorSubject *subject, const char *new_value) {
    free(subject->value);
    if (!new_value) {
        subject->value = NULL;
    } else {
        subject->value = mstrdup(new_value);
        if (subject->value == NULL) {
            printf("Failed to allocate memory for new value\n");
            return;
        }
    }
    if (subject->subscriber_count > 0 && subject->subscribers[0]) subject->subscribers[0](subject->value);
}

void unsubscribe(BehaviorSubject *subject) {
    free(subject->subscribers);
    free(subject->value);
    subject->subscribers = NULL;
    subject->value = NULL;
    subject->subscriber_count = 0;
    subject->capacity = 0;
}
