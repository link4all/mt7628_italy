#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <map>
#include <queue>

#include <glib-2.0/glib.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <status.h>

using namespace std;
using namespace rapidjson;

class DeviceBase {
public:
    string device_serial;
    string device_model;
    string hw_revision;
    string sw_version;
    string os_type;
    string soc_type;
    string vendor;
    string extend;
    string sub_vendor;
    string project_prefix;
    bool is_virtualdevice;

public:
    void ToJson(Document &Json) {
        Json.SetObject();

        Value device_serial_(device_serial.c_str(), device_serial.length(), Json.GetAllocator());
        Json.AddMember("device-serial", device_serial_, Json.GetAllocator());

        Value device_model_(device_model.c_str(), device_model.length(), Json.GetAllocator());
        Json.AddMember("device-model", device_model_, Json.GetAllocator());

        Value hw_revision_(hw_revision.c_str(), hw_revision.length(), Json.GetAllocator());
        Json.AddMember("hw-revision", hw_revision_, Json.GetAllocator());

        Value sw_version_(sw_version.c_str(), sw_version.length(), Json.GetAllocator());
        Json.AddMember("sw-version", sw_version_, Json.GetAllocator());

        Value os_type_(os_type.c_str(), os_type.length(), Json.GetAllocator());
        Json.AddMember("os-type", os_type_, Json.GetAllocator());

        Value soc_type_(soc_type.c_str(), soc_type.length(), Json.GetAllocator());
        Json.AddMember("soc-type", soc_type_, Json.GetAllocator());

        Value vendor_(vendor.c_str(), vendor.length(), Json.GetAllocator());
        Json.AddMember("vendor", vendor_, Json.GetAllocator());

        Value extend_(extend.c_str(), extend.length(), Json.GetAllocator());
        Json.AddMember("extend", extend_, Json.GetAllocator());

        Value sub_vendor_(sub_vendor.c_str(), sub_vendor.length(), Json.GetAllocator());
        Json.AddMember("sub-vendor", sub_vendor_, Json.GetAllocator());

        Value project_prefix_(project_prefix.c_str(), project_prefix.length(), Json.GetAllocator());
        Json.AddMember("project-prefix", project_prefix_, Json.GetAllocator());

        Value is_virtualdevice_;
        is_virtualdevice_.SetBool(is_virtualdevice);
        Json.AddMember("is-virtualdevice", is_virtualdevice_, Json.GetAllocator());
    }
};

class DeviceStatus {
public:
    string loadavg;
    string freemem;
    string uptime;
    string wanip;
    vector<struct RTUErrors> rtu_error_count;

public:
    void ToJson(Document &Json) {
        Json.SetObject();

        Value loadavg_(loadavg.c_str(), loadavg.length(), Json.GetAllocator());
        Json.AddMember("system-load", loadavg_, Json.GetAllocator());

        Value freemem_(freemem.c_str(), freemem.length(), Json.GetAllocator());
        Json.AddMember("free-memory", freemem_, Json.GetAllocator());

        Value uptime_(uptime.c_str(), uptime.length(), Json.GetAllocator());
        Json.AddMember("uptime", uptime_, Json.GetAllocator());

        Value wanip_(wanip.c_str(), wanip.length(), Json.GetAllocator());
        Json.AddMember("wan-ip-address", wanip_, Json.GetAllocator());

        Value rtu_ecnts(kArrayType);
        for (vector<struct RTUErrors>::iterator it = rtu_error_count.begin(); it != rtu_error_count.end(); it++) {
            Value item(kObjectType);
            Value port_((*it).Port.c_str(), (*it).Port.length(), Json.GetAllocator());
            item.AddMember("serial-port", port_, Json.GetAllocator());
            item.AddMember("error-count", (*it).Error, Json.GetAllocator());
            item.AddMember("crc-error-count", (*it).CRC, Json.GetAllocator());
            item.AddMember("rx-timeout-count", (*it).Timeout, Json.GetAllocator());
            rtu_ecnts.PushBack(item, Json.GetAllocator());
        }

        Json.AddMember("serial-port-status", rtu_ecnts, Json.GetAllocator());
    }
};

class DevicePing {
public:
    string device_serial;

public:
    void ToJson(Document &Json) {
        Json.SetObject();

        Value device_serial_(device_serial.c_str(), device_serial.length(), Json.GetAllocator());
        Json.AddMember("device-serial", device_serial_, Json.GetAllocator());
    }
};

class DeviceCmdFeedback {
public:
    string id;
    string status;
    string results;

public:
    void ToJson(Document &Json) {
        Json.SetObject();

        Value id_(id.c_str(), id.length(), Json.GetAllocator());
        Json.AddMember("id", id_, Json.GetAllocator());

        Value status_(status.c_str(), status.length(), Json.GetAllocator());
        Json.AddMember("status", status_, Json.GetAllocator());

        Value results_(results.c_str(), results.length(), Json.GetAllocator());
        Json.AddMember("results", results_, Json.GetAllocator());
    }

public:
    DeviceCmdFeedback(): id(""), status("UNKNOWN"), results("") {}
};

struct TerminalInfo {
    string  Serial;
    string  Port;
    string  Model;
    int     BaudRate;
};

struct FC810Status {
    uint8_t type;
    uint8_t version;
    uint8_t model;
    uint8_t number_of_ports;
    uint8_t used_ports;
    uint8_t room_temperature;
//    uint8_t link_status:1;
//    uint8_t reserved1:1;
//    uint8_t reserved2:1;
//    uint8_t mode_status:1;
//    uint8_t balance_alarm:1;
//    uint8_t disabled:1;
//    uint8_t allow_charging:1;
//    uint8_t factory_mode:1;
    uint8_t unit_status1;
//    uint8_t reserved3:1;
//    uint8_t reserved4:1;
//    uint8_t reserved5:1;
//    uint8_t reserved6:1;
//    uint8_t reserved7:1;
//    uint8_t reserved8:1;
//    uint8_t reserved9:1;
//    uint8_t valve_on:1;
    uint8_t unit_status2;
    uint8_t reserved10;
    uint8_t reserved11;
    uint8_t fcu_counter_int32;
    uint8_t fcu_counter_int24;
    uint8_t fcu_counter_int16;
    uint8_t fcu_counter_int8;
    uint8_t fcu_counter_frac;
//    uint8_t fcu_in_counting:1;
//    uint8_t has_floor_heating:1;
//    uint8_t valve2_on:1;
//    uint8_t valve1_on:1;
//    uint8_t reserved12:1;
//    uint8_t fcu_high_speed:1;
//    uint8_t fcu_medium_speed:1;
//    uint8_t fcu_low_speed:1;
    uint8_t fcu_status;
//    uint8_t sensor_error:1;
//    uint8_t datastore_error:1;
//    uint8_t reserved13:1;
//    uint8_t reserved14:1;
//    uint8_t invalid_port_params:1;
//    uint8_t vavle_force_on:1;
//    uint8_t speed_ctrl_line_error:1;
//    uint8_t fan_offline:1;
    uint8_t faults;
    uint8_t reserved15;
};

struct FC810Counter {
    uint8_t monthly_fee_int32;
    uint8_t monthly_fee_int24;
    uint8_t monthly_fee_int16;
    uint8_t monthly_fee_int8;
    uint8_t current_balance_int32;
    uint8_t current_balance_int24;
    uint8_t current_balance_int16;
    uint8_t current_balance_int8;
    uint8_t allow_query;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
    uint8_t reserved4;
    uint8_t reserved5;
    uint8_t link_down_threshold;
    uint8_t number_of_port;
    uint8_t fcu_motor_power;
    uint8_t high_capacity_h;
    uint8_t high_capacity_l;
    uint8_t medium_capacity_h;
    uint8_t medium_capacity_l;
    uint8_t low_capacity_h;
    uint8_t low_capacity_l;
    uint8_t floor_heating_capacity_h;
    uint8_t floor_heating_capacity_l;
    uint8_t reserved6;
};

struct FC810Location {
    uint8_t building;
    uint8_t unit;
    uint8_t room_h;
    uint8_t room_l;
};

struct FC810Settings {
    uint8_t power_on;
    uint8_t setting_mode;
    uint8_t setting_speed;
    uint8_t setting_temperature_h;
    uint8_t setting_temperature_l;
    uint8_t setting_key_lock;
    uint8_t setting_op_at_acback;
    uint8_t setting_enable_fanonly;
    uint8_t setting_temperature_ul;
    uint8_t setting_temperature_dl;
};

struct FC810DataSet {
    struct FC810Status      Status;
    struct FC810Counter     Counter;
    struct FC810Location    Location;
    struct FC810Settings    Settings;

    void ToJson(Value &Json, Document::AllocatorType &Allocator) {
        // Status
        Json.AddMember("type", Status.type, Allocator);
        Json.AddMember("version", Status.version, Allocator);
        Json.AddMember("model", Status.model, Allocator);
        Json.AddMember("number_of_ports", Status.number_of_ports, Allocator);
        Json.AddMember("used_ports", Status.used_ports, Allocator);
        Json.AddMember("room_temperature", Status.room_temperature, Allocator);
        Json.AddMember("link_status", (Status.unit_status1 >> 7) & 0x1, Allocator);
        Json.AddMember("mode_status", (Status.unit_status1 >> 4) & 0x1, Allocator);
        Json.AddMember("balance_alarm", (Status.unit_status1 >> 3) & 0x1, Allocator);
        Json.AddMember("disabled", (Status.unit_status1 >> 2) & 0x1, Allocator);
        Json.AddMember("allow_charging", (Status.unit_status1 >> 1) & 0x1, Allocator);
        Json.AddMember("factory_mode", (Status.unit_status1 >> 0) & 0x1, Allocator);
        Json.AddMember("valve_on", (Status.unit_status2 >> 0) & 0x1, Allocator);
        int fcu_counter_int = (Status.fcu_counter_int32 << 24) | (Status.fcu_counter_int24 << 16) | (Status.fcu_counter_int16 << 8) | (Status.fcu_counter_int8);
        Json.AddMember("fcu_counter_int", fcu_counter_int, Allocator);
        Json.AddMember("fcu_counter_frac", Status.fcu_counter_frac, Allocator);
        Json.AddMember("fcu_in_counting", (Status.fcu_status >> 7) & 0x1, Allocator);
        Json.AddMember("has_floor_heating", (Status.fcu_status >> 6) & 0x1, Allocator);
        Json.AddMember("valve2_on", (Status.fcu_status >> 5) & 0x1, Allocator);
        Json.AddMember("valve1_on", (Status.fcu_status >> 4) & 0x1, Allocator);
        Json.AddMember("fcu_high_speed", (Status.fcu_status >> 2) & 0x1, Allocator);
        Json.AddMember("fcu_medium_speed", (Status.fcu_status >> 1) & 0x1, Allocator);
        Json.AddMember("fcu_low_speed", (Status.fcu_status >> 0) & 0x1, Allocator);
        Json.AddMember("sensor_error", (Status.faults >> 7) & 0x1, Allocator);
        Json.AddMember("datastore_error", (Status.faults >> 6) & 0x1, Allocator);
        Json.AddMember("invalid_port_params", (Status.faults >> 3) & 0x1, Allocator);
        Json.AddMember("vavle_force_on", (Status.faults >> 2) & 0x1, Allocator);
        Json.AddMember("speed_ctrl_line_error", (Status.faults >> 1) & 0x1, Allocator);
        Json.AddMember("fan_offline", (Status.faults >> 0) & 0x1, Allocator);

        // Counter
        int monthly_fee_int = (Counter.monthly_fee_int32 << 24) | (Counter.monthly_fee_int24 << 16) | (Counter.monthly_fee_int16 << 8) | (Counter.monthly_fee_int8);
        Json.AddMember("monthly_fee_int", monthly_fee_int, Allocator);
        int current_balance_int = (Counter.current_balance_int32 << 24) | (Counter.current_balance_int24 << 16) | (Counter.current_balance_int16 << 8) | (Counter.current_balance_int32);
        Json.AddMember("current_balance_int", current_balance_int, Allocator);
        Json.AddMember("allow_query", Counter.allow_query, Allocator);
        Json.AddMember("link_down_threshold", Counter.link_down_threshold, Allocator);
        Json.AddMember("number_of_port", Counter.number_of_port, Allocator);
        Json.AddMember("fcu_motor_power", Counter.fcu_motor_power, Allocator);
        int high_capacity = (Counter.high_capacity_h << 8) | (Counter.high_capacity_l);
        Json.AddMember("high_capacity", high_capacity, Allocator);
        int medium_capacity = (Counter.medium_capacity_h << 8) | (Counter.medium_capacity_l);
        Json.AddMember("medium_capacity", medium_capacity, Allocator);
        int low_capacity = (Counter.low_capacity_h << 8) | (Counter.low_capacity_l);
        Json.AddMember("low_capacity", low_capacity, Allocator);
        int floor_heating_capacity = (Counter.floor_heating_capacity_h << 8) | (Counter.floor_heating_capacity_l);
        Json.AddMember("floor_heating_capacity", floor_heating_capacity, Allocator);

        // Location
        Json.AddMember("building", Location.building, Allocator);
        Json.AddMember("unit", Location.unit, Allocator);
        int room = (Location.room_h << 8) |(Location.room_l);
        Json.AddMember("room", room, Allocator);

        // Settings
        Json.AddMember("setting_power_on", Settings.power_on, Allocator);
        Json.AddMember("setting_mode", Settings.setting_mode, Allocator);
        Json.AddMember("setting_speed", Settings.setting_speed, Allocator);
        int setting_temperature = (Settings.setting_temperature_h << 8) | (Settings.setting_temperature_l);
        Json.AddMember("setting_temperature", setting_temperature, Allocator);
        Json.AddMember("setting_key_lock", Settings.setting_key_lock, Allocator);
        Json.AddMember("setting_op_at_acback", Settings.setting_op_at_acback, Allocator);
        Json.AddMember("setting_enable_fanonly", Settings.setting_enable_fanonly, Allocator);
        Json.AddMember("setting_temperature_ul", Settings.setting_temperature_ul, Allocator);
        Json.AddMember("setting_temperature_dl", Settings.setting_temperature_dl, Allocator);
    }
};

struct FC890Status {
    uint8_t type;
    uint8_t version;
    uint8_t model;
    uint8_t auto_mode;
    uint8_t is_running;
    uint8_t sys_status;
//    uint8_t link_status:1;
//    uint8_t reserved1:1;
//    uint8_t in_counting:1;
//    uint8_t mode_status:1;
//    uint8_t balance_alarm:1;
//    uint8_t disabled:1;
//    uint8_t allow_charging:1;
//    uint8_t factory_mode:1;
    uint8_t unit_status;
    uint8_t counter_total_int32;
    uint8_t counter_total_int24;
    uint8_t counter_total_int16;
    uint8_t counter_total_int8;
    uint8_t counter_total_frac;
    uint8_t tsensor_type;
    uint8_t tsensor_value_h;
    uint8_t tsensor_value_l;
    uint8_t hsensor_type;
    uint8_t hsensor_value_h;
    uint8_t hsensor_value_l;
    uint8_t extsensor_type;
    uint8_t extsensor_value_h;
    uint8_t extsensor_value_l;
    uint8_t psensor_type;
    uint8_t psensor_value_h;
    uint8_t psensor_value_l;
    uint8_t water_valve_aperture;
    uint8_t humidifier_on;
    uint8_t humidifier_aperture;
    uint8_t fan_motor_on;
    uint8_t vffan_freq_h;
    uint8_t vffan_freq_l;
    uint8_t fau_aperture;
    uint8_t fau_on;
//    uint8_t reserved2:1;
//    uint8_t E7:1;
//    uint8_t E6:1;
//    uint8_t E5:1;
//    uint8_t E4:1;
//    uint8_t E3:1;
//    uint8_t E2:1;
//    uint8_t E1:1;
    uint8_t fault1;
//    uint8_t E18:1;
//    uint8_t reserved3:1;
//    uint8_t reserved4:1;
//    uint8_t reserved5:1;
//    uint8_t reserved6:1;
//    uint8_t reserved7:1;
//    uint8_t reserved8:1;
//    uint8_t E11:1;
    uint8_t fault2;
    uint8_t reserved9;
    uint8_t reserved10;
};

struct FC890Counter {
    uint8_t monthly_fee_int32;
    uint8_t monthly_fee_int24;
    uint8_t monthly_fee_int16;
    uint8_t monthly_fee_int8;
    uint8_t current_balance_int32;
    uint8_t current_balance_int24;
    uint8_t current_balance_int16;
    uint8_t current_balance_int8;
    uint8_t allow_query;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
    uint8_t reserved4;
    uint8_t reserved5;
    uint8_t link_down_threshold;
    uint8_t number_of_port;
    uint8_t fcu_motor_power;
    uint8_t swap_capacity_h;
    uint8_t swap_capacity_l;
    uint8_t reserved6;
};

struct FC890Settings {
    uint8_t setting_power_on;
    uint8_t setting_mode;
    uint8_t setting_allow_ventilation;
    uint8_t setting_cooling_temperature_h;
    uint8_t setting_cooling_temperature_l;
    uint8_t setting_heating_temperature_h;
    uint8_t setting_heating_temperature_l;
    uint8_t setting_temperature_ul;
    uint8_t setting_temperature_dl;
    uint8_t setting_humidity_h;
    uint8_t setting_humidity_l;
    uint8_t setting_humidity_ul;
    uint8_t setting_humidity_dl;
    uint8_t setting_static_pressure_h;
    uint8_t setting_static_pressure_l;
    uint8_t setting_static_pressure_ul_h;
    uint8_t setting_static_pressure_ul_l;
    uint8_t setting_static_pressure_dl_h;
    uint8_t setting_static_pressure_dl_l;
    uint8_t setting_key_lock;
    uint8_t setting_op_at_acback;
    uint8_t reserved1;
    uint8_t reserved2;
    uint8_t reserved3;
    uint8_t reserved4;
    uint8_t setting_tsensor_range;
    uint8_t setting_has_hsensor;
    uint8_t setting_hsensor_range;
    uint8_t setting_has_ext_tsensor;
    uint8_t setting_ext_tsensor_range;
    uint8_t setting_has_psensor;
    uint8_t setting_psensor_range_h;
    uint8_t setting_psensor_range_l;
    uint8_t setting_wv_type;
    uint8_t setting_fwv_distance;
    uint8_t setting_rwv_otype;
    uint8_t setting_wv_Kp;
    uint8_t setting_wv_Ki;
    uint8_t setting_wv_dz;
    uint8_t setting_wv_period;
    uint8_t setting_wv_offaperture;
    uint8_t setting_has_humidity_ctrl;
    uint8_t setting_humidifier_type;
    uint8_t setting_humidifier_otype;
    uint8_t setting_humidifier_Kp;
    uint8_t setting_humidifier_Ki;
    uint8_t setting_humidifier_dz;
    uint8_t setting_humidifier_period;
    uint8_t setting_vffan_en;
    uint8_t setting_vffan_dfl;
    uint8_t setting_vffan_otype;
    uint8_t setting_vffan_var;
    uint8_t setting_vffan_Kp;
    uint8_t setting_vffan_Ki;
    uint8_t setting_vffan_dz;
    uint8_t setting_vffan_period;
    uint8_t setting_fau_type;
    uint8_t setting_fau_init_aperture;
    uint8_t setting_fau_distance;
    uint8_t setting_fan_delay;
};

struct FC890DataSet {
    struct FC890Status      Status;
    struct FC890Counter     Counter;
    struct FC890Settings    Settings;

    void ToJson(Value &Json, Document::AllocatorType &Allocator) {
        // Status
        Json.AddMember("type", Status.type, Allocator);
        Json.AddMember("version", Status.version, Allocator);
        Json.AddMember("model", Status.model, Allocator);
        Json.AddMember("auto_mode", Status.auto_mode, Allocator);
        Json.AddMember("is_running", Status.is_running, Allocator);
        Json.AddMember("sys_status", Status.sys_status, Allocator);
        Json.AddMember("link_status", (Status.unit_status >> 7) & 0x1, Allocator);
        Json.AddMember("in_counting", (Status.unit_status >> 5) & 0x1, Allocator);
        Json.AddMember("mode_status", (Status.unit_status >> 4) & 0x1, Allocator);
        Json.AddMember("balance_alarm", (Status.unit_status >> 3) & 0x1, Allocator);
        Json.AddMember("disabled", (Status.unit_status >> 2) & 0x1, Allocator);
        Json.AddMember("allow_charging", (Status.unit_status >> 1) & 0x1, Allocator);
        Json.AddMember("factory_mode", (Status.unit_status >> 0) & 0x1, Allocator);
        int counter_total_int = (Status.counter_total_int32 << 24) | (Status.counter_total_int24 << 16) | (Status.counter_total_int16 << 8) | (Status.counter_total_int8);
        Json.AddMember("counter_total_int", counter_total_int, Allocator);
        Json.AddMember("counter_total_frac", Status.counter_total_frac, Allocator);
        Json.AddMember("tsensor_type", Status.tsensor_type, Allocator);
        int tsensor_value = (Status.tsensor_value_h << 8) | (Status.tsensor_value_l);
        Json.AddMember("tsensor_value", tsensor_value, Allocator);
        Json.AddMember("hsensor_type", Status.hsensor_type, Allocator);
        int hsensor_value = (Status.hsensor_value_h << 8) | (Status.hsensor_value_l);
        Json.AddMember("hsensor_value", hsensor_value, Allocator);
        Json.AddMember("extsensor_type", Status.extsensor_type, Allocator);
        int extsensor_value = (Status.extsensor_value_h << 8) | (Status.extsensor_value_l);
        Json.AddMember("extsensor_value", extsensor_value, Allocator);
        Json.AddMember("psensor_type", Status.psensor_type, Allocator);
        int psensor_value = (Status.psensor_value_h << 8) | (Status.psensor_value_l);
        Json.AddMember("psensor_value", psensor_value, Allocator);
        Json.AddMember("water_valve_aperture", Status.water_valve_aperture, Allocator);
        Json.AddMember("humidifier_on", Status.humidifier_on, Allocator);
        Json.AddMember("humidifier_aperture", Status.humidifier_aperture, Allocator);
        Json.AddMember("fan_motor_on", Status.fan_motor_on, Allocator);
        int vffan_freq = (Status.vffan_freq_h << 8) | (Status.vffan_freq_l);
        Json.AddMember("vffan_freq", vffan_freq, Allocator);
        Json.AddMember("fau_aperture", Status.fau_aperture, Allocator);
        Json.AddMember("fau_on", Status.fau_on, Allocator);
        Json.AddMember("E7", (Status.fault1 >> 6) & 0x1, Allocator);
        Json.AddMember("E6", (Status.fault1 >> 5) & 0x1, Allocator);
        Json.AddMember("E5", (Status.fault1 >> 4) & 0x1, Allocator);
        Json.AddMember("E4", (Status.fault1 >> 3) & 0x1, Allocator);
        Json.AddMember("E3", (Status.fault1 >> 2) & 0x1, Allocator);
        Json.AddMember("E2", (Status.fault1 >> 1) & 0x1, Allocator);
        Json.AddMember("E1", (Status.fault1 >> 0) & 0x1, Allocator);
        Json.AddMember("E18", (Status.fault2 >> 7) & 0x1, Allocator);
        Json.AddMember("E11", (Status.fault2 >> 0) & 0x1, Allocator);

        // Counter
        int monthly_fee_int = (Counter.monthly_fee_int32 << 24) | (Counter.monthly_fee_int24 << 16) | (Counter.monthly_fee_int16 << 8) | (Counter.monthly_fee_int8);
        Json.AddMember("monthly_fee_int", monthly_fee_int, Allocator);
        int current_balance_int = (Counter.current_balance_int32 << 24) | (Counter.current_balance_int24 << 16) | (Counter.current_balance_int16 << 8) | (Counter.current_balance_int32);
        Json.AddMember("current_balance_int", current_balance_int, Allocator);
        Json.AddMember("allow_query", Counter.allow_query, Allocator);
        Json.AddMember("link_down_threshold", Counter.link_down_threshold, Allocator);
        Json.AddMember("number_of_port", Counter.number_of_port, Allocator);
        Json.AddMember("fcu_motor_power", Counter.fcu_motor_power, Allocator);
        int swap_capacity = (Counter.swap_capacity_h << 8) | (Counter.swap_capacity_l);
        Json.AddMember("swap_capacity", swap_capacity, Allocator);

        // Settings
        Json.AddMember("setting_power_on", Settings.setting_power_on, Allocator);
        Json.AddMember("setting_mode", Settings.setting_mode, Allocator);
        Json.AddMember("setting_allow_ventilation", Settings.setting_allow_ventilation, Allocator);
        int setting_cooling_temperature = (Settings.setting_cooling_temperature_h << 8) | (Settings.setting_cooling_temperature_l);
        Json.AddMember("setting_cooling_temperature", setting_cooling_temperature, Allocator);
        int setting_heating_temperature = (Settings.setting_heating_temperature_h << 8) | (Settings.setting_heating_temperature_l);
        Json.AddMember("setting_heating_temperature", setting_heating_temperature, Allocator);
        Json.AddMember("setting_temperature_ul", Settings.setting_temperature_ul, Allocator);
        Json.AddMember("setting_temperature_dl", Settings.setting_temperature_dl, Allocator);
        int setting_humidity = (Settings.setting_humidity_h << 8) | (Settings.setting_humidity_l);
        Json.AddMember("setting_humidity", setting_humidity, Allocator);
        Json.AddMember("setting_humidity_ul", Settings.setting_humidity_ul, Allocator);
        Json.AddMember("setting_humidity_dl", Settings.setting_humidity_dl, Allocator);
        int setting_static_pressure = (Settings.setting_static_pressure_h << 8) | (Settings.setting_static_pressure_l);
        Json.AddMember("setting_static_pressure", setting_static_pressure, Allocator);
        int setting_static_pressure_ul = (Settings.setting_static_pressure_ul_h << 8) | (Settings.setting_static_pressure_ul_l);
        Json.AddMember("setting_static_pressure_ul", setting_static_pressure_ul, Allocator);
        int setting_static_pressure_dl = (Settings.setting_static_pressure_dl_h << 8) | (Settings.setting_static_pressure_dl_l);
        Json.AddMember("setting_static_pressure_dl", setting_static_pressure_dl, Allocator);
        Json.AddMember("setting_key_lock", Settings.setting_key_lock, Allocator);
        Json.AddMember("setting_op_at_acback", Settings.setting_op_at_acback, Allocator);
        Json.AddMember("setting_tsensor_range", Settings.setting_tsensor_range, Allocator);
        Json.AddMember("setting_has_hsensor", Settings.setting_has_hsensor, Allocator);
        Json.AddMember("setting_hsensor_range", Settings.setting_hsensor_range, Allocator);
        Json.AddMember("setting_has_ext_tsensor", Settings.setting_has_ext_tsensor, Allocator);
        Json.AddMember("setting_ext_tsensor_range", Settings.setting_ext_tsensor_range, Allocator);
        Json.AddMember("setting_has_psensor", Settings.setting_has_psensor, Allocator);
        int setting_psensor_range = (Settings.setting_psensor_range_h << 8) | (Settings.setting_psensor_range_l);
        Json.AddMember("setting_psensor_range", setting_psensor_range, Allocator);
        Json.AddMember("setting_wv_type", Settings.setting_wv_type, Allocator);
        Json.AddMember("setting_fwv_distance", Settings.setting_fwv_distance, Allocator);
        Json.AddMember("setting_rwv_otype", Settings.setting_rwv_otype, Allocator);
        Json.AddMember("setting_wv_Kp", Settings.setting_wv_Kp, Allocator);
        Json.AddMember("setting_wv_Ki", Settings.setting_wv_Ki, Allocator);
        Json.AddMember("setting_wv_dz", Settings.setting_wv_dz, Allocator);
        Json.AddMember("setting_wv_period", Settings.setting_wv_period, Allocator);
        Json.AddMember("setting_wv_offaperture", Settings.setting_wv_offaperture, Allocator);
        Json.AddMember("setting_has_humidity_ctrl", Settings.setting_has_humidity_ctrl, Allocator);
        Json.AddMember("setting_humidifier_type", Settings.setting_humidifier_type, Allocator);
        Json.AddMember("setting_humidifier_otype", Settings.setting_humidifier_otype, Allocator);
        Json.AddMember("setting_humidifier_Kp", Settings.setting_humidifier_Kp, Allocator);
        Json.AddMember("setting_humidifier_Ki", Settings.setting_humidifier_Ki, Allocator);
        Json.AddMember("setting_humidifier_dz", Settings.setting_humidifier_dz, Allocator);
        Json.AddMember("setting_humidifier_period", Settings.setting_humidifier_period, Allocator);
        Json.AddMember("setting_vffan_en", Settings.setting_vffan_en, Allocator);
        Json.AddMember("setting_vffan_dfl", Settings.setting_vffan_dfl, Allocator);
        Json.AddMember("setting_vffan_otype", Settings.setting_vffan_otype, Allocator);
        Json.AddMember("setting_vffan_var", Settings.setting_vffan_var, Allocator);
        Json.AddMember("setting_vffan_Kp", Settings.setting_vffan_Kp, Allocator);
        Json.AddMember("setting_vffan_Ki", Settings.setting_vffan_Ki, Allocator);
        Json.AddMember("setting_vffan_dz", Settings.setting_vffan_dz, Allocator);
        Json.AddMember("setting_vffan_period", Settings.setting_vffan_period, Allocator);
        Json.AddMember("setting_fau_type", Settings.setting_fau_type, Allocator);
        Json.AddMember("setting_fau_init_aperture", Settings.setting_fau_init_aperture, Allocator);
        Json.AddMember("setting_fau_distance", Settings.setting_fau_distance, Allocator);
        Json.AddMember("setting_fan_delay", Settings.setting_fan_delay, Allocator);
    }
};

#include <math.h>
struct HM01DataSet {
    struct {
        uint8_t cooling_capacity[4];
        uint8_t cooling_capacity_unit;
        uint8_t heating_capacity[4];
        uint8_t heating_capacity_unit;
        uint8_t power[4];
        uint8_t power_unit;
        uint8_t instant_discharge[4];
        uint8_t instant_discharge_unit;
        uint8_t discharge[4];
        uint8_t discharge_unit;
        uint8_t in_temperature[3];
        uint8_t cycle_temperature[3];
        uint8_t uptime[3];
        uint8_t time[3];
        uint8_t date[4];
        uint8_t status_l;
        uint8_t status_h;
    } Counter;

private:
    unsigned int get_uint_from_bcd(uint8_t *bcd, int len) {
        unsigned int result = 0;
        for (int i = 0; i < len; i++) {
            result += (bcd[i] & 0xf) * pow(10, 2*(i+1)-2);
            result += ((bcd[i] >> 4) & 0xf) * pow(10, 2*(i+1)-1);
        }
        return result;
    }

public:
    void ToJson(Value &Json, Document::AllocatorType &Allocator) {
        Json.AddMember("cooling_capacity", get_uint_from_bcd(Counter.cooling_capacity, 4), Allocator);
        Json.AddMember("heating_capacity", get_uint_from_bcd(Counter.heating_capacity, 4), Allocator);
        Json.AddMember("power", get_uint_from_bcd(Counter.power, 4), Allocator);
        Json.AddMember("instant_discharge", get_uint_from_bcd(Counter.instant_discharge, 4), Allocator);
        Json.AddMember("discharge", get_uint_from_bcd(Counter.discharge, 4), Allocator);
        Json.AddMember("in_temperature", (float)get_uint_from_bcd(Counter.in_temperature, 3) / 100.0, Allocator);
        Json.AddMember("cycle_temperature", (float)get_uint_from_bcd(Counter.cycle_temperature, 3) / 100.0, Allocator);
        Json.AddMember("uptime", get_uint_from_bcd(Counter.uptime, 3), Allocator);
        Json.AddMember("time", get_uint_from_bcd(Counter.time, 3), Allocator);
        Json.AddMember("date", get_uint_from_bcd(Counter.date, 4), Allocator);
        Json.AddMember("status", Counter.status_h, Allocator);
    }
};

struct FC109RDataSet {
    struct {
        uint16_t model;
        uint16_t version;
        uint8_t power_on;
        uint8_t valve_on;
        uint8_t time_hour;
        uint8_t time_minute;
        uint8_t time_second;
        uint8_t speed;
        uint8_t mode;
        uint8_t temperature;
        uint8_t room_temperature;
        uint8_t key_lock;
        uint8_t temperature_ul;
        uint8_t temperature_dl;
        uint8_t op_at_acback;
        uint8_t antifreezing;
    } Data;

public:
    void ToJson(Value &Json, Document::AllocatorType &Allocator) {
        Json.AddMember("model", Data.model, Allocator);
        Json.AddMember("version", Data.version, Allocator);
        Json.AddMember("power_on", Data.power_on, Allocator);
        Json.AddMember("valve_on", Data.valve_on, Allocator);
        Json.AddMember("time_hour", Data.time_hour, Allocator);
        Json.AddMember("time_minute", Data.time_minute, Allocator);
        Json.AddMember("time_second", Data.time_second, Allocator);
        Json.AddMember("speed", Data.speed, Allocator);
        Json.AddMember("mode", Data.mode, Allocator);
        Json.AddMember("temperature", Data.temperature, Allocator);
        Json.AddMember("room_temperature", Data.room_temperature, Allocator);
        Json.AddMember("key_lock", Data.key_lock, Allocator);
        Json.AddMember("temperature_ul", Data.temperature_ul, Allocator);
        Json.AddMember("temperature_dl", Data.temperature_dl, Allocator);
        Json.AddMember("op_at_acback", Data.op_at_acback, Allocator);
        Json.AddMember("antifreezing", Data.antifreezing, Allocator);
    }
};

struct H3UDataSet {
public:
    struct {
        int opcode;
        int addr;
        int bytes;
        vector<int> data;
    } Data;

public:
    void ToJson(Value &Json, Document::AllocatorType &Allocator)
    {
        Json.AddMember("opcode", Data.opcode, Allocator);
        Json.AddMember("addr", Data.addr, Allocator);
        Json.AddMember("bytes", Data.bytes, Allocator);

        Value data_array(kArrayType);
        for (vector<int>::iterator it = Data.data.begin(); it != Data.data.end(); it++) {
            Value item(kNumberType);
            item.SetInt(*it);
            data_array.PushBack(item, Allocator);
        }
        Json.AddMember("data", data_array, Allocator);
    }
};

struct DataSet {
    long LastUpdate;
    string Model;
    string Serial;
    struct {
        struct FC810DataSet FC810;
        struct FC890DataSet FC890;
        struct HM01DataSet  HM01;
        struct FC109RDataSet FC109;
        struct H3UDataSet H3U;
    } Data;

public:
    DataSet() { Model = ""; Serial = ""; memset(&Data, 0, sizeof(Data)); }

};

typedef vector<struct DataSet>      TerminalDataArray;
typedef map<string, struct DataSet> TerminalDataSet;
typedef queue<struct DataSet>       TerminalUpdated;

#endif // MODELS_H
