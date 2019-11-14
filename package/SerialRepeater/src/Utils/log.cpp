#include "log.h"

#include <queue>
#include <string>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include <glib-2.0/glib.h>

using namespace std;

#define LOG_FILE_PATH   "/tmp/SerialRepeater.txt"
#define LOG_PREFIX      "[SerialRepeater] "

static pthread_mutex_t  logfile_lock;
static int              logfile_fd;
int                     to_stderr;
guint                   log_hid;

static inline int timestamp(char *strtime, uint32_t max)
{
    if (!strtime) {
        return -1;
    }

    time_t timer = {0};
    struct tm utc_time = {0};

    time(&timer);
    gmtime_r(&timer, &utc_time);

    snprintf(strtime, max,
             "%d-%02d-%02d %02d.%02d.%02d",
             utc_time.tm_year + 1900,
             utc_time.tm_mon + 1,
             utc_time.tm_mday,
             utc_time.tm_hour,
             utc_time.tm_min,
             utc_time.tm_sec
             );

    return 0;
}

static void log_handler(const gchar *log_domain, GLogLevelFlags log_level, const gchar *message, gpointer user_data)
{
    char ts[128] = {0};
    timestamp(ts, 128);

    string record;
    record.reserve(512);

    record += LOG_PREFIX;
    record += ts;
    record += ": ";
    record += message;
    record += "\n";

    pthread_mutex_lock(&logfile_lock);
    write(logfile_fd, record.c_str(), record.length());
    pthread_mutex_unlock(&logfile_lock);

    if (to_stderr) fprintf(stderr, "%s", record.c_str());
}

void log_refresh()
{
    pthread_mutex_lock(&logfile_lock);
    close(logfile_fd);
    struct stat lfstat = {0};
    stat(LOG_FILE_PATH, &lfstat);
    if (lfstat.st_size > 1024*1024) remove(LOG_FILE_PATH);
    logfile_fd = open(LOG_FILE_PATH, O_WRONLY|O_APPEND|O_CREAT|O_NONBLOCK, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
    pthread_mutex_unlock(&logfile_lock);
}

int log_open(int debug)
{
    log_hid = g_log_set_handler(NULL,
                                (GLogLevelFlags) (G_LOG_LEVEL_DEBUG|G_LOG_LEVEL_INFO|G_LOG_LEVEL_MESSAGE|G_LOG_LEVEL_WARNING|G_LOG_FLAG_FATAL|G_LOG_FLAG_RECURSION),
                                log_handler,
                                NULL);
    to_stderr = debug;
    logfile_fd = open(LOG_FILE_PATH, O_WRONLY|O_APPEND|O_CREAT|O_NONBLOCK, S_IWUSR|S_IRUSR|S_IRGRP|S_IROTH);
    logfile_lock = PTHREAD_MUTEX_INITIALIZER;
    return 0;
}

int log_close()
{
    g_log_remove_handler(NULL, log_hid);
    close(logfile_fd);
    return 0;
}
