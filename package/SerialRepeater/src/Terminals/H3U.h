#ifndef H3U_H
#define H3U_H

#include <evloop.h>
#include <map>
#include <queue>

#include <models.h>
#include <tcpclient.h>
#include <collect.h>
#include <status.h>

#include <std_modbus_device.h>

using namespace std;

class H3U {
public:
    queue<StdModbusOperate *> *events;
    map<string, StandardModbusTCP *> *StdModbusDevices;
    EventLoop *ev;
    pthread_t tid;
    long interval;
    long batch_size;
    TerminalDataArray tds;
    WSAPI *WS;
    pthread_mutex_t Lock;

public:
    void Dispatch();
    void Start();
    void Stop();
    void SendData(TerminalDataArray &tda);
    void SetCmd(const char *id, int addr, int op, int num, int i32, uint8_t *i8_array, uint16_t *i16_array);

public:
    H3U(long interval_, long batch_size_, WSAPI *ws_, map<string, StandardModbusTCP *> *devtable);
    ~H3U();
};

#endif // H3U_H
