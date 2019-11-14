#ifndef STATUS_H
#define STATUS_H

#include <string>
#include <vector>

#include <rtu.h>

using namespace std;

struct RTUErrors {
    string Port;
    uint64_t Error;
    uint64_t Timeout;
    uint64_t CRC;
};

string get_status_loadavg();

string get_status_freemem();

string get_status_uptime();

string get_status_wan_ipaddr();

void get_status_rtu_ecnt(vector<RTU *> &rtus, vector<struct RTUErrors> &results);

#endif // STATUS_H
