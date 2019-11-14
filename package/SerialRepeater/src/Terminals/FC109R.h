#ifndef FC109R_H
#define FC109R_H

#include <evloop.h>
#include <map>
#include <queue>

#include <models.h>
#include <tcpclient.h>
#include <collect.h>
#include <status.h>

using namespace std;

class PVPoN1 {
public:
    TCPClient *client;
    queue<PVPoN1_Event *> *events;
    queue<PVPoN1_Event *> *pending;
    EventLoop *ev;
    pthread_t tid;
    long interval;
    long batch_size;
    TerminalDataSet tds;
    WSAPI *WS;
    pthread_mutex_t Lock;

public:
    int FC109R_SetMulti(vector<string> &ids, struct DataSet &dataset);
    int FC109R_SetSingle(string id, struct DataSet &dataset);
    void Dispatch();
    void Start();
    void Stop();

public:
    PVPoN1(long interval_, long batch_size_, WSAPI *ws_);
    ~PVPoN1();
};

#endif // FC109R_H
