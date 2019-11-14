#ifndef COLLECT_H
#define COLLECT_H

#include <string>
#include <deque>
#include <vector>
#include <queue>
#include <map>
#include <pthread.h>

#include <glib-2.0/glib.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <evloop.h>
#include <rtu.h>
#include <models.h>
#include <wsapi.h>

#define COLLECTOR_EV_TRIGGER_SENDALL    0
#define COLLECTOR_EV_TRIGGER_SINGLE     1

struct CollectorEvent {
    int         Type;
    string      Serial;
};

class Collector
{
private:
    pthread_mutex_t                 mutex;
    EventLoop                       *ev;
    pthread_t                       tid;

public:
    long                            interval;
    long                            batch_size;
    RTU                             *attached_rtu;
    Terminals                       *terminals;
    TerminalDataSet                 tds;
    queue<struct CollectorEvent>    queued_ev;
    WSAPI                           *WS;

public:
    Collector(long interval_, long batch_size_, RTU *rtu_, Terminals *terminals_, WSAPI *ws_);
    ~Collector();

    void Start();
    void Stop();
    void Dispatch();
    void SendImmeAll();
    void SendImmeSingle(const char *serial);

    void Lock() { pthread_mutex_lock(&mutex); }
    void Unlock() { pthread_mutex_unlock(&mutex); }
};

void CollectorRenderJson(struct DataSet &ds, Document *Json);

#endif // COLLECT_H
