#ifndef BEHAVIOR_SUBJECT_H
#define BEHAVIOR_SUBJECT_H

#include <stddef.h>

typedef void (*subscriber_cb)(const char *);

typedef struct {
    char *value;
    subscriber_cb *subscribers;
    size_t subscriber_count;
    size_t capacity;
} BehaviorSubject;

void init_behavior_subject(BehaviorSubject *subject, const char *initial_value);
void subscribe(BehaviorSubject *subject, subscriber_cb callback);
void next(BehaviorSubject *subject, const char *new_value);
void unsubscribe(BehaviorSubject *subject);

#endif  // BEHAVIOR_SUBJECT_H
