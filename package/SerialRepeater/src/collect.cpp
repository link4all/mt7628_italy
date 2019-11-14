#include "collect.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <Terminals/FC810.h>
#include <Terminals/FC890.h>
#include <Terminals/HM01.h>

#include <delay.hpp>

static void send_batch_data(Collector *cl)
{
    Document *Json = new Document;
    Json->SetArray();

    for (Terminals::iterator it = cl->terminals->begin(); it != cl->terminals->end(); it++) {
        TerminalInfo &t = it->second;
        RTU *vrtu = cl->attached_rtu;
        if (!vrtu->CheckPortnum(atoi(t.Port.c_str()))) continue;

        if (t.Model == "FC810") {
            for (int cmdid = 0; cmdid <= FC810_CMD_READALL_SETTINGS; cmdid++) {
                vrtu->Lock();
                vrtu->ReadAll("FC810", (const unsigned char *)t.Serial.c_str(), cmdid);
                string framebuf = vrtu->Framebuf;
                vrtu->Unlock();
                msleep(160);
                if (vrtu->CheckStatus()) {
                    FC810_Update(t.Serial.c_str(), (unsigned char *)framebuf.c_str(), cmdid, cmdid>=FC810_CMD_READALL_SETTINGS?true:false, cl->tds, Json);
                }
            }
        } else if (t.Model == "FC890") {
            for (int cmdid = 0; cmdid <= FC890_CMD_READALL_SETTINGS; cmdid++) {
                vrtu->Lock();
                vrtu->ReadAll("FC890", (const unsigned char *)t.Serial.c_str(), cmdid);
                string framebuf = vrtu->Framebuf;
                vrtu->Unlock();
                msleep(160);
                if (vrtu->CheckStatus()) {
                    FC890_Update(t.Serial.c_str(), (unsigned char *)framebuf.c_str(), cmdid, cmdid>=FC890_CMD_READALL_SETTINGS?true:false, cl->tds, Json);
                }
            }
        } else if (t.Model == "HM01") {
            vrtu->Lock();
            vrtu->ReadAll("HM01", (const unsigned char *)t.Serial.c_str(), 0);
            string framebuf = vrtu->Framebuf;
            vrtu->Unlock();
            msleep(160);
            if (vrtu->CheckStatus()) {
                HM01_Update(t.Serial.c_str(), (unsigned char *)framebuf.c_str(), true, cl->tds, Json);
            }
        }

        if (cl->batch_size > 0 && Json->Size() >= cl->batch_size) {
            cl->WS->SendData(Json);
            delete Json;
            Json = new Document;
            Json->SetArray();
        }
    }

    if (Json->Size() > 0) cl->WS->SendData(Json);
    delete Json;
}

static void send_single_data(Collector *cl, const char *serial)
{
    Document *Json = new Document;
    Json->SetArray();

    Terminals::iterator it = cl->terminals->find(serial);
    if (it != cl->terminals->end()) {
        TerminalInfo &t = it->second;
        RTU *vrtu = cl->attached_rtu;
        if (!vrtu->CheckPortnum(atoi(t.Port.c_str()))) return;

        if (t.Model == "FC810") {
            for (int cmdid = 0; cmdid <= FC810_CMD_READALL_SETTINGS; cmdid++) {
                vrtu->Lock();
                vrtu->ReadAll("FC810", (const unsigned char *)t.Serial.c_str(), cmdid);
                string framebuf = vrtu->Framebuf;
                vrtu->Unlock();
                msleep(160);
                if (vrtu->CheckStatus()) {
                    FC810_Update(t.Serial.c_str(), (unsigned char *)framebuf.c_str(), cmdid, cmdid>=FC810_CMD_READALL_SETTINGS?true:false, cl->tds, Json);
                }
            }
        } else if (t.Model == "FC890") {
            for (int cmdid = 0; cmdid <= FC890_CMD_READALL_SETTINGS; cmdid++) {
                vrtu->Lock();
                vrtu->ReadAll("FC890", (const unsigned char *)t.Serial.c_str(), cmdid);
                string framebuf = vrtu->Framebuf;
                vrtu->Unlock();
                msleep(160);
                if (vrtu->CheckStatus()) {
                    FC890_Update(t.Serial.c_str(), (unsigned char *)framebuf.c_str(), cmdid, cmdid>=FC890_CMD_READALL_SETTINGS?true:false, cl->tds, Json);
                }
            }
        } else if (t.Model == "HM01") {
            vrtu->Lock();
            vrtu->ReadAll("HM01", (const unsigned char *)t.Serial.c_str(), 0);
            string framebuf = vrtu->Framebuf;
            vrtu->Unlock();
            msleep(160);
            if (vrtu->CheckStatus()) {
                HM01_Update(t.Serial.c_str(), (unsigned char *)framebuf.c_str(), true, cl->tds, Json);
            }
        }
    }

    if (Json->Size() > 0) cl->WS->SendData(Json);
    delete Json;
}

static void ev_handler(evutil_socket_t fd, short flags, void* args)
{
    Collector *cl = (Collector *)args;
    static unsigned int call_cnt = 0;

    call_cnt++;

    if (call_cnt >= cl->interval) {
        call_cnt = 0;
        struct CollectorEvent ev;
        ev.Type = COLLECTOR_EV_TRIGGER_SENDALL;
        cl->queued_ev.push(ev);
    }

    while (!cl->queued_ev.empty()) {
        struct CollectorEvent &ev = cl->queued_ev.front();
        if (ev.Type == COLLECTOR_EV_TRIGGER_SENDALL) {
            send_batch_data(cl);
        } else if (ev.Type == COLLECTOR_EV_TRIGGER_SINGLE) {
            send_single_data(cl, ev.Serial.c_str());
        }
        cl->queued_ev.pop();
    }
}

static void *ev_dispatcher(void *args)
{
    Collector *cl = (Collector *)args;
    cl->Dispatch();
}

void CollectorRenderJson(struct DataSet &ds, Document *Json)
{
    Value item(kObjectType);

    Value device_serial_(ds.Serial.c_str(), ds.Serial.length(), Json->GetAllocator());
    item.AddMember("device-serial", device_serial_, Json->GetAllocator());

    Value device_model_(ds.Model.c_str(), ds.Model.length(), Json->GetAllocator());
    item.AddMember("device-model", device_model_, Json->GetAllocator());

    Value data(kObjectType);
    if (ds.Model == "FC810") {
        ds.Data.FC810.ToJson(data, Json->GetAllocator());
    } else if (ds.Model == "FC890") {
        ds.Data.FC890.ToJson(data, Json->GetAllocator());
    } else if (ds.Model == "HM01") {
        ds.Data.HM01.ToJson(data, Json->GetAllocator());
    }
    item.AddMember("data", data, Json->GetAllocator());

    Json->PushBack(item, Json->GetAllocator());
}

Collector::Collector(long interval_, long batch_size_, RTU *rtu_, Terminals *terminals_, WSAPI *ws_)
{
    interval = interval_;
    batch_size = batch_size_;
    attached_rtu = rtu_;
    terminals = terminals_;
    WS = ws_;
    tds.clear();
    mutex = PTHREAD_MUTEX_INITIALIZER;
    ev = new EventLoop;
}

Collector::~Collector()
{
    Stop();
}

void Collector::Start()
{
    pthread_create(&tid, NULL, ev_dispatcher, (void *)this);
    pthread_detach(tid);
}

void Collector::Stop()
{
    ev->Stop();
}

void Collector::Dispatch()
{
    ev->CreateTimerEvent(ev_handler, 1, 0, this);
    ev->Start();
}

void Collector::SendImmeAll()
{
    struct CollectorEvent ev;
    ev.Type = COLLECTOR_EV_TRIGGER_SENDALL;
    queued_ev.push(ev);
}

void Collector::SendImmeSingle(const char *serial)
{
    struct CollectorEvent ev;
    ev.Type = COLLECTOR_EV_TRIGGER_SINGLE;
    ev.Serial = serial;
    queued_ev.push(ev);
}
