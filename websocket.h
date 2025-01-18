#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <libwebsockets.h>

struct per_session_data {
    struct lws *wsi;
};

extern struct lws_context *context;
extern struct lws *client_wsi;

int callback_websocket(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
void send_message_to_client(const char *key);
int init_websocket_server(int port);
void destroy_websocket_server();

extern struct lws_protocols protocols[];

#endif  // WEBSOCKET_H
