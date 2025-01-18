#include "websocket.h"

#include <libwebsockets.h>

struct lws_context *context;
struct lws *client_wsi = NULL;

int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len) {
    struct per_session_data *psd = (struct per_session_data *)user;
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
                                    },
                                    {NULL, NULL, 0, 0}};

int init_websocket_server(int port) {
    struct lws_context_creation_info info;
    memset(&info, 0, sizeof(info));
    info.port = port;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    static const struct lws_protocol_vhost_options headers[] = {{NULL, NULL, "Cache-Control", "no-cache, no-store"},
                                                                {NULL, NULL, NULL, NULL}};
    static const struct lws_protocol_vhost_options vhost_options[] = {{NULL, headers, "websocket-server", ""},
                                                                      {NULL, NULL, NULL, NULL}};
    info.pvo = vhost_options;

    context = lws_create_context(&info);
    if (!context) {
        perror("Failed to create context\n");
        return -1;
    }
    return 0;
}

void destroy_websocket_server() {
    if (context) {
        lws_context_destroy(context);
    }
}
