#include <dirent.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <linux/input.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

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
};

const char *get_key_name(int key_code, bool shift_pressed) {
    if (key_code < sizeof(key_map) / sizeof(key_map[0]) && key_map[key_code].normal != NULL) {
        return shift_pressed ? key_map[key_code].shifted : key_map[key_code].normal;
    }
    return "UNKNOWN";
}

int main() {
    DIR *dir;
    struct dirent *ent;
    char keyboard_path[256] = "";
    bool keyboard_found = false;
    bool shift_pressed = false;
    bool ctrl_pressed = false;
    bool meta_pressed = false;
    bool alt_pressed = false;

    if ((dir = opendir("/dev/input/by-id")) != NULL) {
        while ((ent = readdir(dir)) != NULL) {
            if (strstr(ent->d_name, "kbd") || strstr(ent->d_name, "keyboard")) {
                snprintf(keyboard_path, sizeof(keyboard_path), "/dev/input/by-id/%s", ent->d_name);
                printf("Finding candidate keyboard device: %s\n", keyboard_path);
                keyboard_found = true;
                break;
            }
        }
        closedir(dir);
    } else {
        perror("Directory /dev/input/by-id cannot be accessed");
        return EXIT_FAILURE;
    }

    if (!keyboard_found) {
        printf("No keyboard device found\n");
        return EXIT_FAILURE;
    }

    printf("Typing from: %s\n", keyboard_path);

    int fd = open(keyboard_path, O_RDONLY);
    if (fd == -1) {
        perror("Cannot open keyboard device");
        return EXIT_FAILURE;
    }

    struct input_event event;
    ssize_t bytes_read;

    while (true) {
        bytes_read = read(fd, &event, sizeof(event));
        if (bytes_read == sizeof(event)) {
            if (event.type == EV_KEY) {
                // Update status key modifier
                if (event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT) {
                    shift_pressed = event.value;
                } else if (event.code == KEY_LEFTCTRL || event.code == KEY_RIGHTCTRL) {
                    ctrl_pressed = event.value;
                } else if (event.code == KEY_LEFTMETA || event.code == KEY_RIGHTMETA) {
                    meta_pressed = event.value;
                } else if (event.code == KEY_LEFTALT || event.code == KEY_RIGHTALT) {
                    alt_pressed = event.value;
                }
                // Handle key press
                if (event.value == 1) {
                    const char *key_name = get_key_name(event.code, shift_pressed);
                    if (strcmp(key_name, "UNKNOWN") != 0) {
                        printf("  ");
                        if (shift_pressed) printf("Shift + ");
                        if (ctrl_pressed) printf("Ctrl + ");
                        if (meta_pressed) printf("Meta + ");
                        if (alt_pressed) printf("Alt + ");
                        printf("%s\n", key_name);
                    }
                }
                // Handle key release
                // else if (event.value == 0) {
                //     // printf("Released: %u\n", event.code);
                // }
            }
        } else if (bytes_read == -1) {
            perror("Error reading from keyboard device");
            break;
        }
    }

    close(fd);

    return 0;
}
