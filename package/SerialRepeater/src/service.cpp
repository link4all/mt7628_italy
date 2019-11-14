#include "service.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <pthread.h>
#include <string.h>

#include <common_defs.h>
#include <baseinfo.h>
#include <http.h>
#include <evloop.h>
#include <models.h>
#include <wsapi.h>
#include <mqtt.h>
#include <rtu.h>
#include <collect.h>
#include <delay.hpp>
#include <persistent.h>
#include <notification_service_routines.h>

#include <Terminals/FC810.h>
#include <Terminals/FC890.h>
#include <Terminals/FC109R.h>
#include <Terminals/H3U.h>
#include <std_modbus_device.h>

using namespace std;

static EventLoop                        *Ev;
static pthread_t                        Tid;
static int                              RegStatus = 0;

WSAPI                                   *WS;
MQTTClient                              *MQ;
PVPoN1                                  *FC109R = NULL;
vector<RTU *>                           RTUs;
vector<Collector *>                     Collectors;
map<string, StandardModbusTCP *>        StdModbusDevices;
Terminals                               Terminal;
DataStorage                             DS("/lib/SRDB.storage");
H3U                                     *H3Us = NULL;
StdModbusAttrTab                        ModbusAttrs;

static int send_register()
{
    // Register
    DeviceBase base;
    base.device_model = DeviceModel;
    base.device_serial = DeviceSerialNumber;
    base.extend = DeviceExtend;
    base.hw_revision = DeviceHWRevision;
    base.is_virtualdevice = DeviceIsVirtualDevice;
    base.os_type = DeviceOSType;
    base.soc_type = DeviceSoCType;
    base.sw_version = DeviceSWVersion;
    base.vendor = DeviceVendor;
    base.sub_vendor = SubVendor;
    base.project_prefix = ProjectPrefix;
    return WS->Register(base);
}

static void send_keepalive(evutil_socket_t fd, short flags, void* args)
{
    DevicePing ka;
    ka.device_serial = DeviceSerialNumber;
    WS->SendPing(ka);

    if (RegStatus != 200) {
        RegStatus = send_register();
    }
}

static void send_status(evutil_socket_t fd, short flags, void* args)
{
    DeviceStatus status;
    bi_collect_status(RTUs, status);
    WS->SendStatus(status);
}

static void killer(evutil_socket_t fd, short flags, void* args)
{
#if 1

#endif
}

static void *ev_dispatcher(void *args)
{
    Ev->Start();
}

int init_Service(void)
{
    bi_load_settings();
    bi_read_device_baseinfo();

    Ev = new EventLoop;
    Ev->CreateTimerEvent(send_keepalive, 10, 0, NULL);
    Ev->CreateTimerEvent(send_status, 15, 0, NULL);
    Ev->CreateTimerEvent(killer, 7200, 0, NULL);

    WS = new WSAPI;

    MQ = new MQTTClient(MQTTBroker.c_str(),
                        MQTTPort,
                        30,
                        DeviceSerialNumber.c_str(),
                        MQTTUsername.c_str(),
                        MQTTPassword.c_str(),
                        false,
                        "",
                        "",
                        "",
                        "");

    if (Serial1Enable) RTUs.push_back(new RTU(Serial1Port.c_str(), 0));
    if (Serial2Enable) RTUs.push_back(new RTU(Serial2Port.c_str(), 1));
    if (Serial3Enable) RTUs.push_back(new RTU(Serial3Port.c_str(), 2));
    if (Serial4Enable) RTUs.push_back(new RTU(Serial4Port.c_str(), 3));

    DS.INITDS();

    return 0;
}

const char *test_payload = "{\"function\":\"set-attr\",\"slave-id\":\"192.168.1.250\",\"attrs\":[{\"opcode\":1,\"addr\":[1,2,3,4,5,6,7,8,9,10,15,16,17,21]},{\"opcode\":3,\"addr\":[2000,2001,2002]}]}";
const char *test_payload2 = "{\"function\":\"interaction\",\"slave-id\":\"192.168.1.250\",\"opcode\":3,\"addr\":2000,\"bytes\":3,\"data\":[]}";

void start_Service(void)
{
    RegStatus = send_register();

    DS.GetTerminals(&Terminal);
    DS.GetStdModbusAttrs(&ModbusAttrs);

    // Add RTU Devices
    for (vector<RTU *>::iterator it = RTUs.begin(); it != RTUs.end(); it++) {
        Collector *CL = new Collector(DataInterval, DataBatch, (*it), &Terminal, WS);
        Collectors.push_back(CL);
        (*it)->Start();
        CL->Start();
    }

    // Add FC109R SP Devices
    FC109R = new PVPoN1(DataInterval, DataBatch, WS);
    FC109R->Start();

    // Add StdModbus Devices
    for (Terminals::iterator it = Terminal.begin(); it != Terminal.end(); it++) {
        TerminalInfo &t = it->second;
        if (t.Model == "H3U") {
            g_message("Found a StdModbus device, ip %s", t.Serial.c_str());
            StandardModbusTCP *dev = new StandardModbusTCP(t.Serial.c_str(), 502);
            dev->Connect();
            StdModbusDevices.insert(pair<string, StandardModbusTCP *>(t.Serial.c_str(), dev));
        }
    }
    H3Us = new H3U(DataInterval, DataBatch, WS, &StdModbusDevices);
    H3Us->Start();

    string topic = "/";
    topic += DeviceSerialNumber;
    topic += "/sysctrl";
    MQ->Subscribe(topic.c_str(), 1, syscrtl_nsr);
    topic = "/";
    topic += DeviceSerialNumber;
    topic += "/terminalctrl";
    MQ->Subscribe(topic.c_str(), 1, terminalcrtl_nsr);
    topic = "/broadcast/sysctrl";
    MQ->Subscribe(topic.c_str(), 1, multicast_nsr);
    topic = "/broadcast/terminalctrl";
    MQ->Subscribe(topic.c_str(), 1, multicast_nsr);
    topic = "/";
    topic += DeviceSerialNumber;
    topic += "/modbus-over-http";
    MQ->Subscribe(topic.c_str(), 1, modbus_over_http_nsr);
    MQ->Start();

    pthread_create(&Tid, NULL, ev_dispatcher, NULL);
    pthread_detach(Tid);
}

void stop_Service(void)
{
    Ev->Stop();
    MQ->Stop();
    DS.~DataStorage();
    delete MQ;
    delete Ev;
    delete WS;
}
