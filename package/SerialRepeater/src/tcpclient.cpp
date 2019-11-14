#include "tcpclient.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <glib-2.0/glib.h>

#include "delay.hpp"

static inline long timestamp()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    long nonce = (long long) tv.tv_sec;
    return nonce;
}

static void *ev_dispatcher(void *arg)
{
    TCPClient *client = (TCPClient *)arg;
    client->Dispatch();
}

void on_messages(bufferevent *buf_ev, void *arg)
{
    TCPClient *client = (TCPClient *)arg;
    struct evbuffer *ibuf = bufferevent_get_input(buf_ev);
    int ilen = evbuffer_get_length(ibuf);
    const char *idata = (const char *)evbuffer_pullup(ibuf, ilen);
    PVPoN1_Event *new_ev = new PVPoN1_Event;
    new_ev->ev_type = FC109R_EVENT_RECIEVED_DATA;
    g_string_append_len(new_ev->data, idata, ilen);
    g_string_append_c(new_ev->data, 0);
    evbuffer_drain(ibuf, ilen);
    client->queued_ev->push(new_ev);
}

void on_events(bufferevent *buf_ev, short event, void *arg)
{
    TCPClient *client = (TCPClient *)arg;
    if ((event & BEV_EVENT_EOF) || (event & BEV_EVENT_ERROR)) {
        client->Connected = false;
        bufferevent_disable(buf_ev, EV_READ|EV_PERSIST);
    }

    if (event & BEV_EVENT_CONNECTED) {
        client->Connected = true;
    }
}

void TCPClient::Dispatch()
{
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port.c_str()));
    inet_aton(addr.c_str(), &server_addr.sin_addr);

    buf_ev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_socket_connect(buf_ev, (sockaddr *)&server_addr, sizeof(server_addr));
    bufferevent_setcb(buf_ev, on_messages, NULL, on_events, this);
    bufferevent_enable(buf_ev, EV_READ|EV_PERSIST);

    event_base_loop(base, EVLOOP_NO_EXIT_ON_EMPTY);
}

int TCPClient::Start()
{
    pthread_create(&tid, NULL, ev_dispatcher, (void *)this);
    pthread_detach(tid);
}

int TCPClient::Stop()
{
    event_base_loopbreak(base);

    if (!buf_ev) return -1;
    bufferevent_disable(buf_ev, EV_READ|EV_PERSIST);
    bufferevent_free(buf_ev);
    buf_ev = NULL;
    return 0;
}

int TCPClient::Send(char *data, int len)
{
    static long sent_at = 0;
    pthread_mutex_lock(&buf_lock);
    if (Connected == false) {
        Reconnect();
        long wait_end = timestamp() + 10;
        while (Connected == false && (timestamp() < wait_end)) {
            msleep(100);
        }
        if (Connected == false) {
            pthread_mutex_unlock(&buf_lock);
            return -1;
        }
    }

    while (timestamp()-sent_at < 3);
    int err = bufferevent_write(buf_ev, data, len);
    sent_at = timestamp();
    pthread_mutex_unlock(&buf_lock);
    return err;
}

int TCPClient::Reconnect()
{
    sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(port.c_str()));
    inet_aton(addr.c_str(), &server_addr.sin_addr);

    bufferevent_free(buf_ev);
    buf_ev = NULL;
    buf_ev = bufferevent_socket_new(base, -1, BEV_OPT_CLOSE_ON_FREE);
    bufferevent_socket_connect(buf_ev, (sockaddr *)&server_addr, sizeof(server_addr));
    bufferevent_setcb(buf_ev, on_messages, NULL, on_events, this);
    bufferevent_enable(buf_ev, EV_READ|EV_PERSIST);
}

TCPClient::TCPClient(const char *saddr, const char *sport, queue<PVPoN1_Event *> *queued_ev_)
{
    queued_ev = queued_ev_;
    addr = saddr;
    port = sport;
    base = event_base_new();
    buf_ev = NULL;
    Connected = false;
    buf_lock = PTHREAD_MUTEX_INITIALIZER;
}

TCPClient::~TCPClient()
{
    Stop();
    event_base_free(base);
}
