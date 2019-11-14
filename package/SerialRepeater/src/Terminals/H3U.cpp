#include "H3U.h"

#include "baseinfo.h"

#include <delay.hpp>

extern StdModbusAttrTab ModbusAttrs;

static inline long timestamp()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    long nonce = (long long) tv.tv_sec;
    return nonce;
}

static void render_json(struct DataSet &ds, Document *Json)
{
    Value item(kObjectType);

    Value device_serial_(ds.Serial.c_str(), ds.Serial.length(), Json->GetAllocator());
    item.AddMember("device-serial", device_serial_, Json->GetAllocator());

    Value device_model_(ds.Model.c_str(), ds.Model.length(), Json->GetAllocator());
    item.AddMember("device-model", device_model_, Json->GetAllocator());

    Value data(kObjectType);
    ds.Data.H3U.ToJson(data, Json->GetAllocator());
    item.AddMember("data", data, Json->GetAllocator());

    Json->PushBack(item, Json->GetAllocator());
}

static void ev_handler(evutil_socket_t fd, short flags, void* args)
{
    TerminalDataArray tda;

    H3U *dev = (H3U *)args;
    while (!dev->events->empty()) {
        StdModbusOperate *ev = dev->events->front();
        int rc = ev->Exec();
        if (rc != ev->num) {
            if (ev->retry >= 5) {
                delete ev;
                dev->events->pop();
                continue;
            }
            dev->events->push(ev);
            dev->events->pop();
            continue;
        }
        switch (ev->opcode) {
        case 0x01:
        {
            struct DataSet ds;
            ds.Model = "H3U";
            ds.Serial = ev->dev->device_ipaddr.c_str();
            ds.LastUpdate = timestamp();
            ds.Data.H3U.Data.opcode = ev->opcode;
            ds.Data.H3U.Data.addr = ev->addr;
            ds.Data.H3U.Data.bytes = ev->num;
            for (int i = 0; i < ev->num; i++) {
                ds.Data.H3U.Data.data.push_back(ev->o8_array[i]);
            }
            tda.push_back(ds);
            break;
        }
        case 0x03:
        case 0x04:
        {
            struct DataSet ds;
            ds.Model = "H3U";
            ds.Serial = ev->dev->device_ipaddr.c_str();
            ds.LastUpdate = timestamp();
            ds.Data.H3U.Data.opcode = ev->opcode;
            ds.Data.H3U.Data.addr = ev->addr;
            ds.Data.H3U.Data.bytes = ev->num;
            for (int i = 0; i < ev->num; i++) {
                ds.Data.H3U.Data.data.push_back(ev->o16_array[i]);
            }
            tda.push_back(ds);
            break;
        }
        }
        delete ev;
        dev->events->pop();

        if (tda.size() >= 16) {
            dev->SendData(tda);
            tda.clear();
        }
    }

    dev->SendData(tda);
}

static void report_task(evutil_socket_t fd, short flags, void* args)
{
    H3U *dev = (H3U *)args;
    for (StdModbusAttrTab::iterator it = ModbusAttrs.begin(); it != ModbusAttrs.end(); it++) {
        StdModbusCmdList &cmds = it->second;
        string id = it->first;
        for (StdModbusCmdList::iterator it_cmds = cmds.begin(); it_cmds != cmds.end(); it_cmds++) {
            StdModbusReadCmd &cmd = *it_cmds;
            dev->SetCmd(id.c_str(), cmd.addr, cmd.opcode, cmd.bytes, 0, NULL, NULL);
        }
    }
}

static void *ev_dispatcher(void *args)
{
    H3U *dev = (H3U *)args;
    dev->Dispatch();
}

void H3U::Dispatch()
{
    ev->CreateTimerEvent(ev_handler, 0, 600, this);
    ev->CreateTimerEvent(report_task, interval, 0, this);
    ev->Start();
}

void H3U::Start()
{
    pthread_create(&tid, NULL, ev_dispatcher, (void *)this);
    pthread_detach(tid);
}

void H3U::Stop()
{
    ev->Stop();
}

void H3U::SendData(TerminalDataArray &tda)
{
    Document *Json = new Document;
    Json->SetArray();

    for (TerminalDataArray::iterator it = tda.begin(); it != tda.end(); it++) {
        struct DataSet &ds = *it;
        render_json(ds, Json);
        if (batch_size > 0 && Json->Size() >= batch_size) {
            WS->SendData(Json);
            delete Json;
            Json = new Document;
            Json->SetArray();
        }
    }

    if (Json->Size() > 0) WS->SendData(Json);
    delete Json;
}

void H3U::SetCmd(const char *id, int addr, int op, int num, int i32, uint8_t *i8_array, uint16_t *i16_array)
{
    map<string, StandardModbusTCP *>::iterator it = StdModbusDevices->find(id);
    if (it != StdModbusDevices->end()) {
        StandardModbusTCP *dev = it->second;
        StdModbusOperate *ev = new StdModbusOperate(dev, op, addr, num, i32, i8_array, i16_array);
        events->push(ev);
    }
}

H3U::H3U(long interval_, long batch_size_, WSAPI *ws_, map<string, StandardModbusTCP *> *devtable)
{
    StdModbusDevices = devtable;
    Lock = PTHREAD_MUTEX_INITIALIZER;
    interval = interval_;
    batch_size = batch_size_;
    WS = ws_;
    tds.clear();
    ev = new EventLoop;
    events = new queue<StdModbusOperate *>;
}

H3U::~H3U()
{
    delete events;
    Stop();
}
