#include "FC109R.h"
#include "baseinfo.h"

#include <delay.hpp>

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
    ds.Data.FC109.ToJson(data, Json->GetAllocator());
    item.AddMember("data", data, Json->GetAllocator());

    Json->PushBack(item, Json->GetAllocator());
}

static void msg_handler(GString *msg, PVPoN1 *dev_)
{
    PVPoN1 *dev = (PVPoN1 *)dev_;
    g_string_append_c(msg, 0);
    gsize b64len;
    guchar *b64de = g_base64_decode(msg->str, &b64len);
    g_message("FC109R Message(Base64): %s", msg->str);

    if (b64de) {
        if (b64len > 11 && !memcmp(b64de, "GATDT", 5) && (b64de[7] * 10 + 11) == b64len) {
            // Update Device Status
            for (int i = 11; i < b64len; i = i + 10) {
                guchar id[3] = {0};
                id[0] = b64de[i+0];
                id[1] = b64de[i+1];
                id[2] = b64de[i+2];
                gchar *idb64 = g_base64_encode(id, 3);
                TerminalDataSet::iterator it = dev->tds.find(idb64);
                if (it != dev->tds.end()) {
                    struct DataSet &ds = it->second;
                    ds.LastUpdate = timestamp();
                    ds.Data.FC109.Data.mode = b64de[i+3];
                    ds.Data.FC109.Data.speed = b64de[i+4];
                    ds.Data.FC109.Data.room_temperature = b64de[i+5];
                    ds.Data.FC109.Data.temperature = b64de[i+6];
                    ds.Data.FC109.Data.power_on = b64de[i+7];
                    ds.Data.FC109.Data.key_lock = b64de[i+8];
                    ds.Data.FC109.Data.valve_on = b64de[i+9];
                } else {
                    struct DataSet ds;
                    ds.Model = "FC109R";
                    ds.Serial = idb64;
                    ds.LastUpdate = timestamp();
                    ds.Data.FC109.Data.mode = b64de[i+3];
                    ds.Data.FC109.Data.speed = b64de[i+4];
                    ds.Data.FC109.Data.room_temperature = b64de[i+5];
                    ds.Data.FC109.Data.temperature = b64de[i+6];
                    ds.Data.FC109.Data.power_on = b64de[i+7];
                    ds.Data.FC109.Data.key_lock = b64de[i+8];
                    ds.Data.FC109.Data.valve_on = b64de[i+9];
                    dev->tds.insert(pair<string, struct DataSet>(idb64, ds));
                }
                g_free(idb64);
            }
        }  else if (b64len >= 19 && !memcmp(b64de, "GATDATA", 7)) {
            // Update Single Device
            struct DataSet dataset;
            guchar data[16] = {0};
            memcpy(data, &b64de[10], 9);
            gchar *idb64 = g_base64_encode(data, 3);
            dataset.Model = "FC109R";
            dataset.Serial = idb64;
            dataset.Data.FC109.Data.mode = data[3];
            dataset.Data.FC109.Data.speed = data[4];
            dataset.Data.FC109.Data.room_temperature = data[5];
            dataset.Data.FC109.Data.temperature = data[6];
            dataset.Data.FC109.Data.power_on = data[7];
            dataset.Data.FC109.Data.key_lock = data[8];
            g_free(idb64);
            Document *Json = new Document;
            Json->SetArray();
            render_json(dataset, Json);
            if (Json->Size() > 0) dev->WS->SendData(Json);
            delete Json;
        }
    }

    g_free(b64de);
}

static void ev_handler(evutil_socket_t fd, short flags, void* args)
{
    PVPoN1 *dev = (PVPoN1 *)args;
    while (!dev->events->empty()) {
        PVPoN1_Event *ev = dev->events->front();
        switch (ev->ev_type) {
        case FC109R_EVENT_RECIEVED_DATA:
        {
            msg_handler(ev->data, dev);
            break;
        }
        case FC109R_EVENT_SEND_SINGLE_CTRL:
        {
            dev->client->Send(ev->data->str, ev->data->len);

            PVPoN1_Event *new_ev = new PVPoN1_Event;
            new_ev->ev_type = FC109R_EVENT_SEND_SINGLE_QUERY;
            g_string_append(new_ev->data, "MANREQT");
            g_string_append_len(new_ev->data, &ev->data->str[7], 4);
            g_string_append(new_ev->data, "MMMM");
            dev->events->push(new_ev);

            break;
        }
        case FC109R_EVENT_SEND_SINGLE_QUERY:
        {
            dev->client->Send(ev->data->str, ev->data->len);
            break;
        }
        default:
            break;
        }
        delete ev;
        dev->events->pop();
    }
}

static void report_task(evutil_socket_t fd, short flags, void* args)
{
    PVPoN1 *dev = (PVPoN1 *)args;

    Document *Json = new Document;
    Json->SetArray();

    g_message("--- FC109R Device List ---");
    g_message("ID        Power    Vavle    Speed    Mode    Temperature    RoomTemperature");
    for (TerminalDataSet::iterator it = dev->tds.begin(); it != dev->tds.end(); it++) {
        struct DataSet &ds = it->second;
        render_json(ds, Json);

        if (dev->batch_size > 0 && Json->Size() >= dev->batch_size) {
            dev->WS->SendData(Json);
            delete Json;
            Json = new Document;
            Json->SetArray();
        }

        g_message("%s      %02d       %02d       %02d       %02d      %02d             %02d",
                  ds.Serial.c_str(),
                  ds.Data.FC109.Data.power_on,
                  ds.Data.FC109.Data.valve_on,
                  ds.Data.FC109.Data.speed,
                  ds.Data.FC109.Data.mode,
                  ds.Data.FC109.Data.temperature,
                  ds.Data.FC109.Data.room_temperature);
    }

    if (Json->Size() > 0) dev->WS->SendData(Json);
    delete Json;
}

static void collect_task(evutil_socket_t fd, short flags, void* args)
{
    PVPoN1 *dev = (PVPoN1 *)args;
    dev->client->Send("LSTREQTLLLLLLLL", 15);
}

static void purging_task(evutil_socket_t fd, short flags, void* args)
{

}

static void *ev_dispatcher(void *args)
{
    PVPoN1 *dev = (PVPoN1 *)args;
    dev->Dispatch();
}

int PVPoN1::FC109R_SetMulti(vector<string> &ids, struct DataSet &dataset)
{
    PVPoN1_Event *new_ev = new PVPoN1_Event;
    new_ev->ev_type = FC109R_EVENT_SEND_MULTI_CTRL_CMD;
    g_string_append(new_ev->data, "LSTCODA");
    g_string_append_c(new_ev->data, 1);
    g_string_append_c(new_ev->data, dataset.Data.FC109.Data.mode);
    g_string_append_c(new_ev->data, dataset.Data.FC109.Data.speed);
    g_string_append_c(new_ev->data, 0);
    g_string_append_c(new_ev->data, dataset.Data.FC109.Data.temperature);
    g_string_append_c(new_ev->data, dataset.Data.FC109.Data.power_on);
    g_string_append_c(new_ev->data, dataset.Data.FC109.Data.key_lock);
    g_string_append_c(new_ev->data, 0);
    events->push(new_ev);

    return 0;
}

int PVPoN1::FC109R_SetSingle(string id, struct DataSet &dataset)
{
    struct DataSet params;
    TerminalDataSet::iterator it = tds.find(id);
    if (it != tds.end()) {
        struct DataSet &ds = it->second;
        if (dataset.Data.FC109.Data.mode != 0xFF) {
            params.Data.FC109.Data.mode = dataset.Data.FC109.Data.mode;
        } else {
            params.Data.FC109.Data.mode = ds.Data.FC109.Data.mode;
        }

        if (dataset.Data.FC109.Data.speed != 0xFF) {
            params.Data.FC109.Data.speed = dataset.Data.FC109.Data.speed;
        } else {
            params.Data.FC109.Data.speed = ds.Data.FC109.Data.speed;
        }

        if (dataset.Data.FC109.Data.temperature != 0xFF) {
            params.Data.FC109.Data.temperature = dataset.Data.FC109.Data.temperature;
        } else {
            params.Data.FC109.Data.temperature = ds.Data.FC109.Data.temperature;
        }

        if (dataset.Data.FC109.Data.power_on != 0xFF) {
            params.Data.FC109.Data.power_on = dataset.Data.FC109.Data.power_on;
        } else {
            params.Data.FC109.Data.power_on = ds.Data.FC109.Data.power_on;
        }

        if (dataset.Data.FC109.Data.key_lock != 0xFF) {
            params.Data.FC109.Data.key_lock = dataset.Data.FC109.Data.key_lock;
        } else {
            params.Data.FC109.Data.key_lock = ds.Data.FC109.Data.key_lock;
        }

        PVPoN1_Event *new_ev = new PVPoN1_Event;
        new_ev->ev_type = FC109R_EVENT_SEND_SINGLE_CTRL;
        g_string_append(new_ev->data, "MANCONT");
        g_string_append(new_ev->data, id.c_str());
        g_string_append_c(new_ev->data, params.Data.FC109.Data.mode);
        g_string_append_c(new_ev->data, params.Data.FC109.Data.speed);
        g_string_append_c(new_ev->data, 0);
        g_string_append_c(new_ev->data, params.Data.FC109.Data.temperature);
        g_string_append_c(new_ev->data, params.Data.FC109.Data.power_on);
        g_string_append_c(new_ev->data, params.Data.FC109.Data.key_lock);
        events->push(new_ev);

        return 0;
    } else {
        return -1;
    }
}

void PVPoN1::Dispatch()
{
    ev->CreateTimerEvent(ev_handler, 0, 600, this);
    ev->CreateTimerEvent(report_task, interval, 0, this);
    ev->CreateTimerEvent(collect_task, 5, 0, this);
    ev->CreateTimerEvent(purging_task, 300, 0, this);
    ev->Start();
}

void PVPoN1::Start()
{
    pthread_create(&tid, NULL, ev_dispatcher, (void *)this);
    pthread_detach(tid);
}

void PVPoN1::Stop()
{
    ev->Stop();
}

PVPoN1::PVPoN1(long interval_, long batch_size_, WSAPI *ws_)
{
    Lock = PTHREAD_MUTEX_INITIALIZER;
    interval = interval_;
    batch_size = batch_size_;
    WS = ws_;
    tds.clear();
    ev = new EventLoop;
    events = new queue<PVPoN1_Event *>;
    client = new TCPClient(IpDeviceAddr.c_str(), IpDevicePort.c_str(), events);
    client->Start();
    g_message("Connect to %s:%s.", IpDeviceAddr.c_str(), IpDevicePort.c_str());
}

PVPoN1::~PVPoN1()
{
    client->Stop();
    delete client;
    delete events;
    Stop();
}
