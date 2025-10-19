#ifndef WS_H
#define WS_H

#include <libwebsockets.h>
#include <stdint.h>

#define DEFAULT_PORT 33300

struct per_session_data {
    struct lws *wsi;
};

extern struct lws_context *context;

int init_websocket_server(uint16_t port);
void send_message_to_client(const char *key);
void destroy_websocket_server();

#endif  // WS_H
