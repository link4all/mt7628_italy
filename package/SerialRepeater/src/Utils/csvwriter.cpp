#include "csvwriter.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <getopt.h>
#include <fcntl.h>
#include <unistd.h>

#include <glib-2.0/glib.h>

#include <cpuset.h>

static inline int filename_random(char *strtime, uint32_t max)
{
    if (!strtime) {
        return -1;
    }

    time_t timer = {0};
    struct tm utc_time = {0};

    time(&timer);
    gmtime_r(&timer, &utc_time);

    snprintf(strtime, max,
             "%d%02d%02d.csv",
             utc_time.tm_year + 1900,
             utc_time.tm_mon + 1,
             utc_time.tm_mday
             );

    return 0;
}

static void flush_log(evutil_socket_t fd, short flags, void* args)
{
    CSVLogger *csv = (CSVLogger *)args;
    csv->Flush();
}

static void *period_flush_tasklet(void *args)
{
//    set_thread_affinity(pthread_self(), 0);

    CSVLogger *csv = (CSVLogger *)args;
    csv->Dispatch();
}

string CSVLogger::fromInt32(int n)
{
    char buf[64] = {0};
    snprintf(buf, 64, "%d", n);
    return string(buf);
}

string CSVLogger::fromInt64(long long n)
{
    char buf[64] = {0};
    snprintf(buf, 64, "%lld", n);
    return string(buf);
}

string CSVLogger::fromLInt(long n)
{
    char buf[64] = {0};
    snprintf(buf, 64, "%ld", n);
    return string(buf);
}

string CSVLogger::fromUInt32(unsigned int n)
{
    char buf[64] = {0};
    snprintf(buf, 64, "%d", n);
    return string(buf);
}

string CSVLogger::fromUInt64(unsigned long long n)
{
    char buf[64] = {0};
    snprintf(buf, 64, "%d", n);
    return string(buf);
}

string CSVLogger::fromULInt(unsigned long n)
{
    char buf[64] = {0};
    snprintf(buf, 64, "%d", n);
    return string(buf);
}

string CSVLogger::fromFloat(float n)
{
    char buf[64] = {0};
    snprintf(buf, 64, "%f", n);
    return string(buf);
}

string CSVLogger::fromDouble(double n)
{
    char buf[64] = {0};
    snprintf(buf, 64, "%lf", n);
    return string(buf);
}

string CSVLogger::fromBool(bool v)
{
    char buf[64] = {0};
    snprintf(buf, 64, "%s", v?"True":"False");
    return string(buf);
}

CSVLogger::CSVLogger(const char *file)
{
    if (file) {
        filename = file;
    } else {
        char buf[256] = {0};
        filename_random(buf, 256);
        filename = buf;
    }

    while (!contents.empty()) contents.pop();

    evloop = new EventLoop(2, 0);
    evloop->CreateConstantTimerEvent(flush_log, (void *)this);
}

CSVLogger::CSVLogger(const char *file, csvRowData &header)
{
    if (file) {
        filename = file;
    } else {
        char buf[256] = {0};
        filename_random(buf, 256);
        filename = buf;
    }

    while (!contents.empty()) contents.pop();

    Insert(header);
    Flush();

    evloop = new EventLoop(2, 0);
    evloop->CreateConstantTimerEvent(flush_log, (void *)this);
}

CSVLogger::~CSVLogger()
{
    evloop->Stop();
    delete evloop;
}

void CSVLogger::Insert(csvRowData &data)
{
    string content;
    content.reserve(1024);
    content = "";
    for (csvRowData::const_iterator it = data.begin(); it != data.end(); it++) {
        if (it != data.begin()) content += ",";
        content += (*it);
    }
    content += '\n';

    contents.push(content);
}

void CSVLogger::Flush()
{
    if (contents.empty()) return;

    int log_fd = open(filename.c_str(), O_WRONLY|O_APPEND|O_CREAT|O_NONBLOCK, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
    if (log_fd < 0) return;

    while (!contents.empty()) {
        write(log_fd, contents.front().c_str(), contents.front().length());
        contents.pop();
    }

    close(log_fd);
}

void CSVLogger::Start()
{
    pthread_create(&tid, NULL, period_flush_tasklet, (void *)this);
    pthread_detach(tid);
}

void CSVLogger::Stop()
{
    evloop->Stop();
}

void CSVLogger::Dispatch()
{
    evloop->Start();
}
