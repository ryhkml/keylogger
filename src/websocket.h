#ifndef WEBSOCKET_H
#define WEBSOCKET_H

#include <libwebsockets.h>
#include <stdint.h>

#define DEFAULT_PORT_WS 33300

struct per_session_data {
    struct lws *wsi;
};

extern struct lws_context *context;

void send_message_to_client(const char *key);
int init_websocket_server(uint16_t port);
void destroy_websocket_server();

#endif  // WEBSOCKET_H
