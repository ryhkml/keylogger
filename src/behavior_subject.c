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
        fprintf(stderr, "Failed to allocate memory for subscribers\n");
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
            fprintf(stderr, "Failed to allocate memory for initial value\n");
            free(subject->subscribers);
            subject->subscribers = NULL;
            subject->capacity = 0;
        }
    }
}

void subscribe(BehaviorSubject *subject, subscriber_cb callback) {
    if (!subject->subscribers) {
        fprintf(stderr, "Subject not initialized\n");
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
    if (!new_value) {
        free(subject->value);
        subject->value = NULL;
    } else {
        char *new_value_copy = mstrdup(new_value);
        if (new_value_copy == NULL) {
            fprintf(stderr, "Failed to allocate memory for new value\n");
            return;
        }
        free(subject->value);
        subject->value = new_value_copy;
    }
    // Notify subscribers
    for (size_t i = 0; i < subject->subscriber_count; i++) {
        if (subject->subscribers[i]) {
            subject->subscribers[i](subject->value);
        }
    }
}

void unsubscribe(BehaviorSubject *subject) {
    free(subject->subscribers);
    free(subject->value);
    subject->subscribers = NULL;
    subject->value = NULL;
    subject->subscriber_count = 0;
    subject->capacity = 0;
}
