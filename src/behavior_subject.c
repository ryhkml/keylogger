#include "behavior_subject.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "util.h"

#define INITIAL_CAPACITY 1

void init_behavior_subject(BehaviorSubject *subject, const char *initial_value) {
    subject->capacity = INITIAL_CAPACITY;
    subject->subscribers = malloc(subject->capacity * sizeof(subscriber_cb));
    if (subject->subscribers == NULL) {
        perror("Failed to allocate memory for subscribers");
        exit(EXIT_FAILURE);
    }
    subject->value = mstrdup(initial_value);
    if (subject->value == NULL) {
        perror("Failed to allocate memory for initial value");
        free(subject->subscribers);
        exit(EXIT_FAILURE);
    }
    subject->subscriber_count = 0;
}

void subscribe(BehaviorSubject *subject, subscriber_cb callback) {
    subject->subscribers = realloc(subject->subscribers, (subject->subscriber_count + 1) * sizeof(subscriber_cb));
    if (subject->subscribers == NULL) {
        perror("Failed to allocate memory for subscribers");
        exit(EXIT_FAILURE);
    }
    subject->subscribers[subject->subscriber_count] = callback;
    subject->subscriber_count++;
    // Immediately emit the current value to the new subscriber
    callback(subject->value);
}

void next(BehaviorSubject *subject, const char *new_value) {
    free(subject->value);
    subject->value = mstrdup(new_value);
    if (subject->value == NULL) {
        perror("Failed to allocate memory for new value");
        exit(EXIT_FAILURE);
    }
    subject->subscribers[0](subject->value);
}

void unsubscribe(BehaviorSubject *subject) {
    free(subject->subscribers);
    free(subject->value);
    subject->subscribers = NULL;
    subject->value = NULL;
    subject->subscriber_count = 0;
    subject->capacity = 0;
}
