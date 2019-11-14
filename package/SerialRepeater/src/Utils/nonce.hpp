#ifndef NONCE_HPP
#define NONCE_HPP

#include <stdio.h>
#include <sys/time.h>

static inline long long timestamp_ms()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    long long nonce = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    return nonce;
}

static inline long timestamp()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    long nonce = (long long) tv.tv_sec;
    return nonce;
}

#endif // NONCE_HPP
