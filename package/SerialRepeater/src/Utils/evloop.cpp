#include "evloop.h"

#include <string.h>
#include <stdlib.h>

EventLoop::EventLoop()
{
    base = event_base_new();
    constant_timeout = NULL;
    max_ev = 0;
}

EventLoop::EventLoop(long seconds, long microseconds)
{
    base = event_base_new();

    constant_timeout = (struct timeval *) calloc(1, sizeof(struct timeval));
    constant_timeout->tv_sec = seconds;
    constant_timeout->tv_usec = microseconds;

    struct timeval tv_in = { seconds, microseconds };
    const struct timeval *tv_out;
    tv_out = event_base_init_common_timeout(base, &tv_in);
    memcpy(constant_timeout, tv_out, sizeof(struct timeval));
}

EventLoop::~EventLoop()
{
    ClearEvents();
    event_base_loopexit(base, NULL);
    event_base_free(base);

    if (constant_timeout) {
        free(constant_timeout);
    }
}

void EventLoop::Start()
{
    event_base_dispatch(base);
}

void EventLoop::Stop()
{
    event_base_loopexit(base, NULL);
}

uint64_t EventLoop::CreateTimerEvent(event_callback_fn cb, long seconds, long microseconds, void *args)
{
    max_ev++;

    short flags = EV_TIMEOUT|EV_PERSIST;
    struct timeval timeout = { seconds, microseconds };
    struct event *ev = event_new(base, -1, flags, cb, args);

    EventControlBlock *ecb = new EventControlBlock(max_ev, ev, args, flags, -1);

    if (0 == event_add(ev, &timeout)) {
        evtable.insert(pair<uint64_t, EventControlBlock*>(max_ev, ecb));
        return max_ev;
    } else {
        delete ecb;
        return 0;
    }
}

uint64_t EventLoop::CreateConstantTimerEvent(event_callback_fn cb, void *args)
{
    if (NULL == constant_timeout) return 0;

    max_ev++;
    short flags = EV_TIMEOUT|EV_PERSIST;
    struct event *ev = event_new(base, -1, flags, cb, args);

    EventControlBlock *ecb = new EventControlBlock(max_ev, ev, args, flags, -1);

    if (0 == event_add(ev, constant_timeout)) {
        evtable.insert(pair<uint64_t, EventControlBlock*>(max_ev, ecb));
        return max_ev;
    } else {
        delete ecb;
        return 0;
    }
}

uint64_t EventLoop::CreateFileEvent(event_callback_fn cb, int fd, void *args, short flags)
{
    max_ev++;
    struct event *ev = event_new(base, fd, flags, cb, args);

    EventControlBlock *ecb = new EventControlBlock(max_ev, ev, args, flags, fd);

    if (0 == event_add(ev, NULL)) {
        evtable.insert(pair<uint64_t, EventControlBlock*>(max_ev, ecb));
        return max_ev;
    } else {
        delete ecb;
        return 0;
    }
}

void EventLoop::RemoveEvent(uint64_t id)
{
    EVTABLE::iterator it = evtable.find(id);
    if (it != evtable.end()) {
        EventControlBlock *ecb = (EventControlBlock *)it->second;
        delete ecb;
    }
    evtable.erase(id);
}

EventControlBlock *EventLoop::LookupEvent(uint64_t id)
{
    EVTABLE::iterator it = evtable.find(id);
    if (it != evtable.end()) {
        EventControlBlock *ecb = (EventControlBlock *)it->second;
        return ecb;
    }
    return NULL;
}

void EventLoop::ClearEvents()
{
    for (EVTABLE::iterator it = evtable.begin(); it != evtable.end(); it++) {
        EventControlBlock *ecb = (EventControlBlock *)it->second;
        delete ecb;
    }
}
