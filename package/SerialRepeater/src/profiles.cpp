#include "profiles.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <string>

#include <glib-2.0/glib.h>

#include <sysconf.h>

using namespace std;

string get_ethaddr()
{
    int fd;
    struct ifreq ifr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) return "00:00:00:00:00:00";
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, "eth0", sizeof(ifr.ifr_name)-1);
    if (ioctl(fd, SIOCGIFHWADDR, (char *)&ifr)) {
        close(fd);
        return "00:00:00:00:00:00";
    }
    close(fd);

    int addr_len = strlen("00:00:00:00:00:00");
    char addr[addr_len+1];
    sprintf(addr, "%02x:%02x:%02x:%02x:%02x:%02x",
            (unsigned char)ifr.ifr_hwaddr.sa_data[0],
            (unsigned char)ifr.ifr_hwaddr.sa_data[1],
            (unsigned char)ifr.ifr_hwaddr.sa_data[2],
            (unsigned char)ifr.ifr_hwaddr.sa_data[3],
            (unsigned char)ifr.ifr_hwaddr.sa_data[4],
            (unsigned char)ifr.ifr_hwaddr.sa_data[5]
            );

    return addr;
}

string get_device_sn()
{
    string ethaddr = get_ethaddr();
    gchar *hash = g_compute_checksum_for_string(G_CHECKSUM_SHA256, ethaddr.c_str(), ethaddr.length());
    char sn[17] = {0};
    strncpy(sn, hash, 16);
    g_free(hash);
    return sn;
}

static const char *mon[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sept", "Oct", "Nov", "Dec"};
static const char *day[] = {"Mon", "Tues", "Wed", "Thur", "Fri", "Sat", "Sun"};

string get_systime()
{
    GTimeVal tv = {0};
    g_get_current_time(&tv);
    GDateTime *tm = g_date_time_new_from_timeval_local(&tv);
    int len = strlen("Thur Sept 00 00:00:00 0000");
    char ftime[len+1];
    sprintf(ftime, "%s %s %d %02d:%02d:%02d %04d",
            day[g_date_time_get_day_of_week(tm)-1],
            mon[g_date_time_get_month(tm)-1],
            g_date_time_get_day_of_month(tm),
            g_date_time_get_hour(tm),
            g_date_time_get_minute(tm),
            g_date_time_get_second(tm),
            g_date_time_get_year(tm)
            );
    g_date_time_unref(tm);
    return ftime;
}

string get_key()
{
    return sysconf_get("SerialRepeater.global.api_key");
}

string get_secret()
{
    return sysconf_get("SerialRepeater.global.api_secret");
}

void save_key_pair(const char *key, const char *secret)
{
    sysconf_set("SerialRepeater.global.api_key", key);
    sysconf_set("SerialRepeater.global.api_secret", secret);
}
