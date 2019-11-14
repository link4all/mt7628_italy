#include "baseinfo.h"

#include <profiles.h>
#include <sysconf.h>

#include <glib-2.0/glib.h>

// Setting Parameters
/*
SerialRepeater.ports.serial1_port='/dev/ttyS0'
SerialRepeater.ports.serial2_port='/dev/ttyS1'
SerialRepeater.ports.serial3_port='/dev/ttyS2'
SerialRepeater.ports.serial4_port='/dev/ttyS3'
SerialRepeater.ports.serial1_enable='1'
SerialRepeater.ports.serial2_enable='0'
SerialRepeater.ports.serial3_enable='0'
SerialRepeater.ports.serial4_enable='0'
SerialRepeater.global.api_key='*'
SerialRepeater.global.api_secret='*'
SerialRepeater.global.api_base='http:///'
SerialRepeater.global.mqtt_username='admin'
SerialRepeater.global.mqtt_password='admin'
SerialRepeater.global.mqtt_broker='47.106.71.14'
SerialRepeater.global.mqtt_port='1883'
SerialRepeater.global.data_interval='30'
SerialRepeater.global.data_retry='5'
SerialRepeater.global.data_batch='0'
SerialRepeater.global.sub_vendor='0'
SerialRepeater.global.project_prefix='0'
SerialRepeater.global.ctrl_room_no='101'
*/
string                   Serial1Port;
string                   Serial2Port;
string                   Serial3Port;
string                   Serial4Port;
bool                     Serial1Enable;
bool                     Serial2Enable;
bool                     Serial3Enable;
bool                     Serial4Enable;
string                   APIBase;
string                   MQTTUsername;
string                   MQTTPassword;
string                   MQTTBroker;
int                      MQTTPort;
int                      DataInterval;
int                      DataRetry;
int                      DataBatch;
string                   SubVendor;
string                   ProjectPrefix;
string                   CtrlRoomNumber;
string                   IpDeviceAddr;
string                   IpDevicePort;

// Device Basic Information - DBI
string                   DeviceModel;
string                   DeviceMACAddress;
string                   DeviceSerialNumber;
string                   DeviceHWRevision;
bool                     DeviceIsVirtualDevice;
string                   DeviceOSType;
string                   DeviceSoCType;
string                   DeviceSWVersion;
string                   DeviceVendor;
string                   DeviceExtend;

void bi_load_settings()
{
    Serial1Port = sysconf_get("SerialRepeater.ports.serial1_port");
    Serial2Port = sysconf_get("SerialRepeater.ports.serial2_port");
    Serial3Port = sysconf_get("SerialRepeater.ports.serial3_port");
    Serial4Port = sysconf_get("SerialRepeater.ports.serial4_port");

    string en1 = sysconf_get("SerialRepeater.ports.serial1_enable");
    Serial1Enable = (en1 == "1")?true:false;
    string en2 = sysconf_get("SerialRepeater.ports.serial2_enable");
    Serial2Enable = (en2 == "1")?true:false;
    string en3 = sysconf_get("SerialRepeater.ports.serial3_enable");
    Serial3Enable = (en3 == "1")?true:false;
    string en4 = sysconf_get("SerialRepeater.ports.serial4_enable");
    Serial4Enable = (en4 == "1")?true:false;

    APIBase = sysconf_get("SerialRepeater.global.api_base");
    MQTTUsername = sysconf_get("SerialRepeater.global.mqtt_username");
    MQTTPassword = sysconf_get("SerialRepeater.global.mqtt_password");
    MQTTBroker = sysconf_get("SerialRepeater.global.mqtt_broker");
    string mqtt_port = sysconf_get("SerialRepeater.global.mqtt_port");
    MQTTPort = atoi(mqtt_port.c_str());

    string data_interval = sysconf_get("SerialRepeater.global.data_interval");
    DataInterval = atoi(data_interval.c_str());
    string data_retry = sysconf_get("SerialRepeater.global.data_retry");
    DataRetry = atoi(data_retry.c_str());
    string data_batch = sysconf_get("SerialRepeater.global.data_batch");
    DataBatch = atoi(data_batch.c_str());

    SubVendor = sysconf_get("SerialRepeater.global.sub_vendor");
    ProjectPrefix = sysconf_get("SerialRepeater.global.project_prefix");
    CtrlRoomNumber = sysconf_get("SerialRepeater.global.ctrl_room_no");

    IpDeviceAddr = sysconf_get("SerialRepeater.ipdev.ipdev_addr");
    IpDevicePort = sysconf_get("SerialRepeater.ipdev.ipdev_port");

    g_message("------------------- Parameters -------------------");
    g_message(" Serial1Port :    %s (%d)", Serial1Port.c_str(), Serial1Enable);
    g_message(" Serial2Port :    %s (%d)", Serial2Port.c_str(), Serial2Enable);
    g_message(" Serial3Port :    %s (%d)", Serial3Port.c_str(), Serial3Enable);
    g_message(" Serial4Port :    %s (%d)", Serial4Port.c_str(), Serial4Enable);
    g_message(" APIBase :        %s", APIBase.c_str());
    g_message(" MQTTUsername :   %s", MQTTUsername.c_str());
    g_message(" MQTTPassword :   %s", MQTTPassword.c_str());
    g_message(" MQTTBroker :     %s", MQTTBroker.c_str());
    g_message(" MQTTPort :       %s", mqtt_port.c_str());
    g_message(" DataInterval :   %s", data_interval.c_str());
    g_message(" DataRetry :      %s", data_retry.c_str());
    g_message(" DataBatch :      %s", data_batch.c_str());
    g_message(" SubVendor :      %s", SubVendor.c_str());
    g_message(" ProjectPrefix :  %s", ProjectPrefix.c_str());
    g_message(" CtrlRoomNumber : %s", CtrlRoomNumber.c_str());
    g_message(" IpDeviceAddr :   %s", IpDeviceAddr.c_str());
    g_message(" IpDevicePort :   %s", IpDevicePort.c_str());
    g_message("--------------------------------------------------");
}

void bi_read_device_baseinfo()
{
    DeviceModel = "SR-101";
    DeviceMACAddress = get_ethaddr();
    DeviceSerialNumber = get_device_sn();
    DeviceHWRevision = "Rev.1";
    DeviceIsVirtualDevice = false;
    DeviceOSType = "Linux";
    DeviceSoCType = "ARMv7-A";
    DeviceSWVersion = "1.0";
    DeviceVendor = "ZOSHANG";
    DeviceExtend = "";
}

void bi_collect_status(vector<RTU *> &rtus, DeviceStatus &status)
{
    status.loadavg = get_status_loadavg();
    status.freemem = get_status_freemem();
    status.wanip = get_status_wan_ipaddr();
    status.uptime = get_status_uptime();
    get_status_rtu_ecnt(rtus, status.rtu_error_count);
}
