#include "status.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/sysinfo.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <time.h>

#include <sysconf.h>

using namespace std;

string get_status_loadavg()
{
    float l1min, l5min, l15min;

    FILE *fd = fopen("/proc/loadavg", "r");
    if (!fd) return "";

    if (3 != fscanf(fd, "%f %f %f", &l1min, &l5min, &l15min)) {
        fclose(fd);
        return "";
    }
    fclose(fd);

    char loadavg[128] = {0};
    snprintf(loadavg, 128, "%f %f %f", l1min, l5min, l15min);
    return loadavg;
}

string get_status_freemem()
{
    struct sysinfo info;
    sysinfo(&info);
    char freemem[128] = {0};
    snprintf(freemem, 128, "%lu", info.freeram >> 10);
    return freemem;
}

string get_status_uptime()
{
    struct sysinfo info;
    time_t curr_ts, after_boot;
    struct tm uptime;
    sysinfo(&info);
    time(&curr_ts);

    if (curr_ts > info.uptime)
        after_boot = curr_ts - info.uptime;
    else
        after_boot = info.uptime - curr_ts;

    char uptime_s[128] = {0};
    localtime_r(&after_boot, &uptime);
    snprintf(uptime_s, 128, "%04d-%02d-%02d %02d:%02d:%02d",
             uptime.tm_year + 1900,
             uptime.tm_mon + 1,
             uptime.tm_mday,
             uptime.tm_hour,
             uptime.tm_min,
             uptime.tm_sec
             );
    return uptime_s;
}

string get_status_wan_ipaddr()
{
    string ifname = sysconf_get("network.wan.ifname");
    char ipaddr[128] = {0};

    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return ipaddr;
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname.c_str(), sizeof(ifr.ifr_name)-1);
    if (!ioctl(fd, SIOCGIFADDR, &ifr)) {
        struct sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
        strncpy(ipaddr, inet_ntoa(sin->sin_addr), 128);
        close(fd);
        return ipaddr;
    }
    close(fd);

    return ipaddr;
}

void get_status_rtu_ecnt(vector<RTU *> &rtus, vector<struct RTUErrors> &results)
{
    results.clear();

    for (vector<RTU *>::iterator it = rtus.begin(); it != rtus.end(); it++) {
        struct RTUErrors e;
        e.Port = (*it)->port;
        e.CRC = (*it)->CRCErrorCount;
        e.Error = (*it)->ErrorCount;
        e.Timeout = (*it)->RXTimeoutCount;
        results.push_back(e);
    }
}
