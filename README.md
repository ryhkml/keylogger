# Keylogger

![Keylogger screenshot](./sample.png)

## Build and Usage

```sh
make
```

Use `sudo` to run. The keylogger outputs to the `/tmp/.keylogger.log` file. You can also get output via a WebSocket. However, this requires the [libwebsockets](https://libwebsockets.org) library. Install libwebsockets and then run:

```sh
make USE_LIBWEBSOCKETS=1
```

If target device is incorrect. Use the `--dev <PATH>` option to specify a device event. List available devices run `ls -l /dev/input/by-id/`

Example:

```sh
sudo out/keylogger --dev /dev/input/event7
```

There is a list of options available

| Option     | Default Value       | Description                     |
| ---------- | ------------------- | ------------------------------- |
| `--dev`    | `/dev/input/event*` | Specify the device event to use |
| `--printk` |                     | Show keystrokes in terminal     |
| `--port`   | `33300`             | Specify websocket port          |

### WebSocket Secure (wss)

The keylogger server also supports secure WebSocket connections. Set an environment variable with the path to your certificate and private key files.

```sh
# Required
export KEYLOGGER_SSL_CERT_PATH=PATH_TO_CERT.pem
export KEYLOGGER_SSL_KEY_PATH=PATH_TO_KEY.pem
# Optional
export KEYLOGGER_SSL_CA_PATH=
```

```js
// Client
const ws = new WebSocket("wss://localhost:33300");
```

> [!NOTE]
>
> Please note that you also need to add the authority certificate file to the browser you are using.
> This file is usually in the `.crt` format.

### Test

```sh
make test
```

The [libcmocka](https://cmocka.org) library is required for testing.

### Rootless

> [!WARNING]
>
> Rootless method gives you direct access to your keyboard.

To run rootless, follow these steps:

1. Create a udev rules file (for example, /etc/udev/rules.d/90-keylogger.rules)

    ```sh
    echo "SUBSYSTEM==\"input\", OWNER=\"$USER\", MODE=\"0660\"" | sudo tee /etc/udev/rules.d/90-keylogger.rules > /dev/null
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
