#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <unistd.h>
#include <string>
#include <queue>

#include <event.h>
#include <event2/bufferevent.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <glib-2.0/glib.h>

using namespace std;

#define FC109R_EVENT_RECIEVED_DATA          0x01
#define FC109R_EVENT_SEND_SINGLE_QUERY      0x02
#define FC109R_EVENT_SEND_SINGLE_CTRL       0x03
#define FC109R_EVENT_SEND_MULTI_CTRL_CMD    0x04
#define FC109R_EVENT_SEND_MULTI_CTRL_IDS    0x05

class PVPoN1_Event {
public:
    int ev_type;
    string ev_id;
    GString *data;
    PVPoN1_Event *next;

public:
    PVPoN1_Event() { ev_id = ""; data = g_string_sized_new(1500); next = NULL; }
    ~PVPoN1_Event() { g_string_free(data, TRUE); }
};

class TCPClient {
private:
    string addr;
    string port;
    event_base *base;
    bufferevent *buf_ev;
    event *ev;
    pthread_t tid;
    pthread_mutex_t buf_lock;

public:
    bool Connected;
    queue<PVPoN1_Event *> *queued_ev;

public:
    void Dispatch();
    int Start();
    int Stop();
    int Send(char *data, int len);
    int Reconnect();

    TCPClient(const char *addr, const char *port, queue<PVPoN1_Event *> *queued_ev_);
    ~TCPClient();
};

#endif // TCPCLIENT_H
