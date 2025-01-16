# Keylogger

![Keylogger screenshot](./sample.png)

This is an implementation of a keylogger that records keyboard input.
The primary purpose of this code is for demonstration and to understand the concepts involved in creating a keylogger in a Linux environment.

## Build and Usage

```sh
make
```

Use `sudo` to run. The keylogger will display output to a `/tmp/.keylogger.log` file.
If target device is incorrect. Use the `--dev <PATH>` option to specify a device event. List available devices run `ls -l /dev/input/by-id/`

Example:

```sh
sudo out/keylogger --dev /dev/input/event7
```

#### Rootless

> [!WARNING]
>
> Rootless method gives you direct access to your keyboard.
> This method poses a potential security risk. Make sure you only use this program for personal use on your own computer.

To run rootless, follow these steps:

1. Create a udev rules file (for example, /etc/udev/rules.d/90-keylogger.rules)

    ```sh
    echo "SUBSYSTEM==\"input\", OWNER=\"$USER\", MODE=\"0660\"" | sudo tee /etc/udev/90-keylogger.rules > /dev/null
    ```

1. Reload udev rules
    ```sh
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    ```

## Formatter

`~/.clang-format` base from [Google](https://google.github.io/styleguide/cppguide.html)

```
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 120
```

_In Google i trust_
