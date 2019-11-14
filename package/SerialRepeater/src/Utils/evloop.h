#ifndef EVLOOP_H
#define EVLOOP_H

#include <map>
#include <vector>
#include <string>

#include <event.h>
#include <glib-2.0/glib.h>

using namespace std;

class EventControlBlock {
public:
    uint64_t        event_id;
    struct event    *ev;
    void            *args;
    int             call_count;
    short           event_flag;
    int             event_fd;

public:
    EventControlBlock(uint64_t id, struct event *ev, void *args, int event_flag, int event_fd):
        event_id(id),
        ev(ev),
        args(args),
        event_flag(event_flag),
        event_fd(event_fd)
    {
        call_count = 0;
    }

    ~EventControlBlock()
    {
        event_del(ev);
        event_free(ev);
    }
};

typedef map<uint64_t, EventControlBlock*> EVTABLE;

class EventLoop {
private:
    struct event_base* base;
    struct timeval* constant_timeout;
    EVTABLE evtable;
    uint64_t max_ev;

public:
    EventLoop();
    EventLoop(long seconds, long microseconds);    // Create EventManager that support constant timer dispatching
    ~EventLoop();

public:
    void Start();
    void Stop();
    uint64_t CreateTimerEvent           (event_callback_fn cb, long seconds, long microseconds, void* args);
    uint64_t CreateConstantTimerEvent   (event_callback_fn cb, void* args);
    uint64_t CreateFileEvent            (event_callback_fn cb, int fd, void* args, short flag);
    void RemoveEvent                    (uint64_t id);
    EventControlBlock *LookupEvent      (uint64_t id);
    void ClearEvents                    ();
};

#endif // EVLOOP_H
