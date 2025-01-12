# Keylogger

![Keylogger screenshot](./sample.png)

This is an implementation of a keylogger that records keyboard input.
The primary purpose of this code is for demonstration and to understand the concepts involved in creating a keylogger in a Linux environment.

## Build and Usage

```sh
make
```

Use `sudo` to run. The keylogger will display output in the terminal by default. Use the `-w` option to write the output to a `/tmp/.keylogger.log` file.

If target device is incorrect. Use the `-dev <NAME>` option to specify a device. List available devices with `ls -l /dev/input/by-id/`

Example:

```sh
sudo out/keylogger -dev usb-Keychron_Keychron_K11_Pro-event-kbd
```

#### Rootless

> [!WARNING]
>
> Rootless method gives you direct access to your keyboard.
> This method poses a potential security risk. Make sure you only use this program for personal use on your own computer.

To run rootless, follow these steps:

1. Create and add user to groups

    ```sh
    sudo usermod -aG <GROUP_NAME> $USER
    ```

1. Create a udev rules file (for example, /etc/udev/rules.d/90-keylogger.rules). Fill the file with the following rules

    ```txt
    KERNEL=="event*", SUBSYSTEM=="input", ATTRS{idVendor}==<VENDOR_ID>, ATTRS{idProduct}==<PRODUCT_ID>, MODE="0660", GROUP=<GROUP_NAME>
    ```

    To find out the vendor ID and product ID, run the `lsusb` command. The response is as follows:

    ```txt
    Bus ... Device ...: ID <THIS_IS_VENDOR_ID>:<THIS_IS_PRODUCT_ID> ...
    ```

1. Reload udev rules
    ```sh
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    ```
1. Log out, then log back in to apply your group changes

## Formatter

`~/.clang-format` base from [Google](https://google.github.io/styleguide/cppguide.html)

```
BasedOnStyle: Google
IndentWidth: 4
ColumnLimit: 120
```

_In Google i trust_
