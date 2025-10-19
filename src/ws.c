#include "ws.h"

#include <libwebsockets.h>
#include <stdbool.h>
#include <stdlib.h>

#include "keylogger.h"

struct lws_context *context;

static struct lws *client_wsi = NULL;

static int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    (void)in;
    (void)len;
    struct per_session_data *psd = (struct per_session_data *)user;
    switch (reason) {
        case LWS_CALLBACK_ESTABLISHED: {
            printf("Client connected\n");
            psd->wsi = wsi;
            client_wsi = wsi;
            lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_SERVER_WRITEABLE: {
            lws_callback_on_writable(wsi);
            break;
        }
        case LWS_CALLBACK_CLOSED: {
            printf("Client disconnected\n");
            psd->wsi = NULL;
            if (client_wsi == wsi) client_wsi = NULL;
            break;
        }
        default:
            break;
    }
    return 0;
}

static struct lws_protocols protocols[] = {
    {.name = "websocket-server",
     .callback = callback_websocket,
     .per_session_data_size = sizeof(struct per_session_data),
     .rx_buffer_size = 0,
     .id = 0},
    {NULL, NULL, 0, 0, 0, NULL, 0}
};

void send_message_to_client(const char *key) {
    if (!client_wsi || !key) return;

    size_t n = strlen(key);
    if (n >= MAX_KEY_LEN) return;

    unsigned char buf[LWS_PRE + MAX_KEY_LEN];
    unsigned char *p = &buf[LWS_PRE];

    memcpy(p, key, n);
    p[n] = '\0';
    lws_write(client_wsi, p, n, LWS_WRITE_TEXT);
}

int init_websocket_server(uint16_t port) {
    if (port == 0) {
        fprintf(stderr, "Invalid port number\n");
        return -1;
    }

    struct lws_context_creation_info info = {0};
    info.port = port;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;
    info.iface = "127.0.0.1";
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

    static const struct lws_protocol_vhost_options headers[] = {
        {NULL, NULL, "Cache-Control", "no-cache, no-store"},
        {NULL, NULL, NULL,            NULL                }
    };
    static const struct lws_protocol_vhost_options vhost_options[] = {
        {NULL, headers, "websocket-server", ""  },
        {NULL, NULL,    NULL,               NULL}
    };
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
        context = NULL;
    }
    client_wsi = NULL;
}
