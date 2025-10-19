#define NOB_IMPLEMENTATION
#define NOB_STRIP_PREFIX

#include "nob.h"

const char *CFLAGS[] = {
    "-O2",
    "-Wall",
    "-Wextra",
    "-Wformat",
    "-Wformat-security",
    "-std=c17",
    "-fstack-protector-strong",
    "-D_FORTIFY_SOURCE=2",
    "-o",
};

const char *OUT_FILENAMES[] = {
    "nob.old",
    "out/keylogger",
    "out/test_keylogger",
};

#define cflags_size ARRAY_LEN(CFLAGS)
#define out_filenames_size ARRAY_LEN(OUT_FILENAMES)

void default_cmd_append(Cmd *cmd) {
    cmd_append(cmd, "gcc");
    for (int i = 0; i < cflags_size; i++) cmd_append(cmd, CFLAGS[i]);
}

int main(int argc, char **argv) {
    NOB_GO_REBUILD_URSELF(argc, argv);

    Cmd cmd = {0};
    Procs procs = {0};

    for (int i = 0; i < out_filenames_size; i++) remove(OUT_FILENAMES[i]);

    bool test = false;

    for (int i = 0; i < argc; i++) {
        if (strcmp(argv[i], "test") == 0) {
            test = true;
            break;
        }
    }

    if (test) {
        default_cmd_append(&cmd);
        cmd_append(&cmd, "out/test_keylogger", "test/test.c", "test/test_keylogger.c", "src/keylogger.c");
        da_append(&procs, cmd_run_async_and_reset(&cmd));

        if (!procs_wait_and_reset(&procs)) return 1;

        cmd_append(&cmd, "./out/test_keylogger");
        da_append(&procs, cmd_run_async_and_reset(&cmd));

        if (!procs_wait_and_reset(&procs)) return 1;

        return 0;
    }

    default_cmd_append(&cmd);

    cmd_append(&cmd, "out/keylogger", "src/main.c", "src/keylogger.c", "src/ws.c", "-lwebsockets");
    da_append(&procs, cmd_run_async_and_reset(&cmd));

    if (!procs_wait_and_reset(&procs)) return 1;

    return 0;
}
