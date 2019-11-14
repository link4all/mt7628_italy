#ifndef DELAY_HPP
#define DELAY_HPP

#include <stdio.h>
#include <sys/select.h>
#include <time.h>

static inline int msleep(long ms)
{
    struct timeval tv = { 0, ms*1000 };
    return select(0, NULL, NULL, NULL, &tv);
}

static inline int ssleep(long s)
{
    struct timespec ts = { s, 0 };
    return nanosleep(&ts, NULL);
}

static inline int nsleep(long ns)
{
    struct timespec ts = { 0, ns };
    return nanosleep(&ts, NULL);
}

#endif // DELAY_HPP
