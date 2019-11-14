#ifndef BASEINFO_H
#define BASEINFO_H

#include <string>

#include <status.h>
#include <models.h>

using namespace std;

// Setting Parameters
extern string                   Serial1Port;
extern string                   Serial2Port;
extern string                   Serial3Port;
extern string                   Serial4Port;
extern bool                     Serial1Enable;
extern bool                     Serial2Enable;
extern bool                     Serial3Enable;
extern bool                     Serial4Enable;
extern string                   APIBase;
extern string                   MQTTUsername;
extern string                   MQTTPassword;
extern string                   MQTTBroker;
extern int                      MQTTPort;
extern int                      DataInterval;
extern int                      DataRetry;
extern int                      DataBatch;
extern string                   SubVendor;
extern string                   ProjectPrefix;
extern string                   CtrlRoomNumber;
extern string                   IpDeviceAddr;
extern string                   IpDevicePort;

// Device Basic Information - DBI
extern string                   DeviceModel;
extern string                   DeviceMACAddress;
extern string                   DeviceSerialNumber;
extern string                   DeviceHWRevision;
extern bool                     DeviceIsVirtualDevice;
extern string                   DeviceOSType;
extern string                   DeviceSoCType;
extern string                   DeviceSWVersion;
extern string                   DeviceVendor;
extern string                   DeviceExtend;

void bi_load_settings();

void bi_read_device_baseinfo();

void bi_collect_status(vector<RTU *> &rtus, DeviceStatus &status);

#endif // BASEINFO_H
