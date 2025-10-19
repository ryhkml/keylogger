# Keylogger

![Keylogger screenshot](./sample.png)

## Prerequisites
Before you begin, ensure the following library is installed:
1. [libwebsockets](https://libwebsockets.org)

## Build and Usage

```sh
gcc -o nob nob.c
./nob
```

If target device is incorrect. Use the `--dev <PATH>` option to specify a device event. List available devices run `ls -l /dev/input/by-id/`

```sh
sudo out/keylogger --dev /dev/input/event7 --printk
```

There is a list of options available:

```txt
sudo out/keylogger [OPTIONS]

Options:
    --dev     [path]   Specify the device event to use (default: /dev/input/event?)
    --printk           Show keystrokes in terminal
    --port    [port]   Specify websocket port (default: 33300)
    -h,--help          Display help message and exit
```

Want to try using Nix?

```sh
nix run github:ryhkml/keylogger -- --dev /dev/input/event? --printk
```

### WebSocket Secure (wss)

The keylogger server also supports secure WebSocket connections. Set an environment variable with the path to your certificate and private key files.

```sh
# Required
export KEYLOGGER_SSL_CERT_PATH="PATH_TO_CERT.pem"
export KEYLOGGER_SSL_KEY_PATH="PATH_TO_KEY.pem"
# Optional
export KEYLOGGER_SSL_CA_PATH=
```

### Design Your Own Keystrokes

Just create an [index.html](./index.html) file as a client to receive keystrokes from the server.

<video src="https://github.com/user-attachments/assets/75fb5f0e-db1f-4927-b76c-1032effdcd57" controls preload muted></video>

### Test

```sh
./nob test
```

### Rootless

> [!WARNING]
>
> Rootless method gives you direct access to your keyboard.

To run rootless, follow these steps:

1. Create a udev rules file (for example, `/etc/udev/rules.d/90-keylogger.rules`)

    ```sh
    echo "SUBSYSTEM==\"input\", OWNER=\"$USER\", MODE=\"0660\"" | sudo tee /etc/udev/rules.d/90-keylogger.rules > /dev/null
    ```

1. Reload udev rules
    ```sh
    sudo udevadm control --reload-rules
    sudo udevadm trigger
    ```
