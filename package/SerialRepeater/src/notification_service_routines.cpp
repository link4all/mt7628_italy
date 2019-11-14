#include "notification_service_routines.h"

#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "baseinfo.h"
#include "models.h"
#include "collect.h"
#include "persistent.h"
#include "Terminals/FC810.h"
#include "Terminals/FC890.h"
#include <Terminals/FC109R.h>
#include <Terminals/H3U.h>
#include <std_modbus_device.h>

#include <delay.hpp>

using namespace rapidjson;
using namespace std;

extern vector<RTU *>                        RTUs;
extern Terminals                            Terminal;
extern DataStorage                          DS;
extern vector<Collector *>                  Collectors;
extern WSAPI                                *WS;
extern map<string, StandardModbusTCP *>     StdModbusDevices;
extern StdModbusAttrTab                     ModbusAttrs;

struct commandItem {
    string          tid;
    string          model;
    int             port;
    unsigned char   msg[256];
    struct DataSet  ds;
};

static int prepare_target_list(string tid_, vector<struct commandItem> &commandList)
{
    struct commandItem c;
    memset(c.msg, 0, sizeof(c.msg));
    c.ds.Data.FC109.Data.power_on = 0xff;
    c.ds.Data.FC109.Data.speed = 0xff;
    c.ds.Data.FC109.Data.mode = 0xff;
    c.ds.Data.FC109.Data.temperature = 0xff;
    c.ds.Data.FC109.Data.key_lock = 0xff;

    string::size_type at_pos = tid_.find("@");
    char id_p1[128] = {0};
    memcpy(id_p1, tid_.c_str(), at_pos);
    char id_p2[128] = {0};
    strcpy(id_p2, tid_.c_str()+at_pos+1);
    string tid = id_p1;
    string sn = id_p2;

    Terminals::iterator it = Terminal.find(tid);
    if (it == Terminal.end()) {
        g_message("Invalid terminal: [%s]", tid.c_str());
        return -1;
    }

    string model = it->second.Model;
    int portnum = atoi(it->second.Port.c_str());
    if (model == "FC810") {
        if (portnum < RTUs.size()) {
            RTUs[portnum]->Lock();
            RTUs[portnum]->ReadAll("FC810", (const uint8_t *)tid.c_str(), FC810_CMD_READALL_SETTINGS);
            string framebuf = RTUs[portnum]->Framebuf;
            msleep(160);
            RTUs[portnum]->Unlock();
            if (RTUs[portnum]->CheckStatus()) {
                memcpy((void *)&c.msg, (void *)&framebuf.c_str()[8], sizeof(struct FC810Settings));
            } else {
                g_message("Failed reading back from terminal");
                return -2;
            }
        }
    } else if (model == "FC890") {
        if (portnum < RTUs.size()) {
            RTUs[portnum]->Lock();
            RTUs[portnum]->ReadAll("FC890", (const uint8_t *)tid.c_str(), FC890_CMD_READALL_SETTINGS);
            string framebuf = RTUs[portnum]->Framebuf;
            msleep(160);
            RTUs[portnum]->Unlock();
            if (RTUs[portnum]->CheckStatus()) {
                memcpy((void *)&c.msg, (void *)&framebuf.c_str()[8], sizeof(struct FC890Settings));
            } else {
                g_message("Failed reading back from terminal");
                return -2;
            }
        }
    } else if (model == "FC109R") {

    } else {
        g_message("Invalid port number");
        return -3;
    }

    c.tid = tid;
    c.model = model;
    c.port = portnum;
    commandList.push_back(c);

    return 0;
}

static void save_params(string N, string V, vector<struct commandItem> &commandList)
{
    for (vector<struct commandItem>::iterator it = commandList.begin(); it != commandList.end(); it++) {
        string model = (*it).model;
        if (model == "FC810") {
            struct FC810Settings &settings_fc810 = (struct FC810Settings &)(*it).msg;
            uint16_t val = (uint16_t) atoi(V.c_str());
            if (N == "setting_power_on") {
                settings_fc810.power_on = val;
            } else if (N == "setting_mode") {
                settings_fc810.setting_mode = val;
            } else if (N == "setting_enable_fanonly") {
                settings_fc810.setting_enable_fanonly = val;
            } else if (N == "setting_speed") {
                settings_fc810.setting_speed = val;
            } else if (N == "setting_temperature") {
                settings_fc810.setting_temperature_h = val >> 8;
                settings_fc810.setting_temperature_l = val & 0x00ff;
            } else if (N == "setting_key_lock") {
                settings_fc810.setting_key_lock = val;
            } else if (N == "setting_op_at_acback") {
                settings_fc810.setting_op_at_acback = val;
            }
        } else if (model == "FC890") {
            struct FC890Settings &settings_fc890 = (struct FC890Settings &)(*it).msg;
        } else if (model == "FC109R") {
            uint16_t val = (uint16_t) atoi(V.c_str());
            if (N == "power_on") {
                (*it).ds.Data.FC109.Data.power_on = val;
            } else if (N == "speed") {
                (*it).ds.Data.FC109.Data.speed = val;
            } else if (N == "mode") {
                (*it).ds.Data.FC109.Data.mode = val;
            } else if (N == "temperature") {
                (*it).ds.Data.FC109.Data.temperature = val;
            } else if (N == "key_lock") {
                (*it).ds.Data.FC109.Data.key_lock = val;
            }
        }
    }
}

static int prepare_counter_target_list(string tid_, vector<struct commandItem> &commandList)
{
    struct commandItem c;
    memset(c.msg, 0, sizeof(c.msg));

    string::size_type at_pos = tid_.find("@");
    char id_p1[128] = {0};
    memcpy(id_p1, tid_.c_str(), at_pos);
    char id_p2[128] = {0};
    strcpy(id_p2, tid_.c_str()+at_pos+1);
    string tid = id_p1;
    string sn = id_p2;

    Terminals::iterator it = Terminal.find(tid);
    if (it == Terminal.end()) {
        g_message("Invalid terminal: [%s]", tid.c_str());
        return -1;
    }

    string model = it->second.Model;
    int portnum = atoi(it->second.Port.c_str());
    if (model == "FC810") {
        if (portnum < RTUs.size()) {
            RTUs[portnum]->Lock();
            RTUs[portnum]->ReadAll("FC810", (const uint8_t *)tid.c_str(), FC810_CMD_READALL_COUNTER);
            string framebuf = RTUs[portnum]->Framebuf;
            msleep(160);
            RTUs[portnum]->Unlock();
            if (RTUs[portnum]->CheckStatus()) {
                memcpy((void *)&c.msg, (void *)&framebuf.c_str()[8], sizeof(struct FC810Counter));
            } else {
                g_message("Failed reading back from terminal");
                return -2;
            }
        }
    } else if (model == "FC890") {
        if (portnum < RTUs.size()) {
            RTUs[portnum]->Lock();
            RTUs[portnum]->ReadAll("FC890", (const uint8_t *)tid.c_str(), FC890_CMD_READALL_COUNTER);
            string framebuf = RTUs[portnum]->Framebuf;
            msleep(160);
            RTUs[portnum]->Unlock();
            if (RTUs[portnum]->CheckStatus()) {
                memcpy((void *)&c.msg, (void *)&framebuf.c_str()[8], sizeof(struct FC890Counter));
            } else {
                g_message("Failed reading back from terminal");
                return -2;
            }
        }
    } else if (model == "FC109R") {

    } else {
        g_message("Invalid port number");
        return -3;
    }

    c.tid = tid;
    c.model = model;
    c.port = portnum;
    commandList.push_back(c);

    return 0;
}

static void save_conunter_params(string N, string V, vector<struct commandItem> &commandList)
{
    for (vector<struct commandItem>::iterator it = commandList.begin(); it != commandList.end(); it++) {
        string model = (*it).model;
        if (model == "FC810") {
            struct FC810Counter &settings_fc810 = (struct FC810Counter &)(*it).msg;
            uint16_t val = (uint16_t) atoi(V.c_str());
            if (N == "fcu_motor_power") {
                settings_fc810.fcu_motor_power = val;
            } else if (N == "high_capacity") {
                settings_fc810.high_capacity_h = val >> 8;
                settings_fc810.high_capacity_l = val & 0x00ff;
            } else if (N == "medium_capacity") {
                settings_fc810.medium_capacity_h = val >> 8;
                settings_fc810.medium_capacity_l = val & 0x00ff;
            } else if (N == "low_capacity") {
                settings_fc810.low_capacity_h = val >> 8;
                settings_fc810.low_capacity_l = val & 0x00ff;
            } else if (N == "floor_heating_capacity") {
                settings_fc810.floor_heating_capacity_h = val >> 8;
                settings_fc810.floor_heating_capacity_l = val & 0x00ff;
            }
        } else if (model == "FC890") {
            struct FC890Counter &settings_fc890 = (struct FC890Counter &)(*it).msg;
            uint16_t val = (uint16_t) atoi(V.c_str());
            if (N == "fcu_motor_power") {
                settings_fc890.fcu_motor_power = val;
            } else if (N == "swap_capacity") {
                settings_fc890.swap_capacity_h = val >> 8;
                settings_fc890.swap_capacity_l = val & 0x00ff;
            }
        }
    }
}

void syscrtl_nsr(const char *payload)
{
    DeviceCmdFeedback feedback;

    Document Json;
    Json.Parse<kParseStopWhenDoneFlag>(payload, strlen(payload));
    if (Json.HasParseError()) {
        return;
    }

    string id;
    string function;

    Value::ConstMemberIterator itr = Json.FindMember("id");
    if (itr != Json.MemberEnd() && itr->value.IsString()) {
        id = itr->value.GetString();
        feedback.id = id;
    } else {
        feedback.status = "FAILED";
        goto stop;
    }

    itr = Json.FindMember("function");
    if (itr != Json.MemberEnd() && itr->value.IsString()) {
        function = itr->value.GetString();
    }

    if (function == "add-terminal") {
        itr = Json.FindMember("args");
        if (itr != Json.MemberEnd() && itr->value.IsArray()) {
            string serial;
            string model;
            string baudrate;
            string port = "0";
            const Value &args = Json["args"];
            for (unsigned int i = 0; i < args.Size(); i++) {
                const Value &var = args[i];
                if (var.IsObject()) {
                    string N, V;
                    Value::ConstMemberIterator itr_var = var.FindMember("N");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        N = itr_var->value.GetString();
                    }
                    itr_var = var.FindMember("V");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        V = itr_var->value.GetString();
                    }

                    if (N == "tid") serial = V.c_str();
                    if (N == "model") model = V.c_str();
                    if (N == "baud") baudrate = V.c_str();
                }
            }

            for (vector<RTU *>::iterator it = RTUs.begin(); it != RTUs.end(); it++) {
                if ((*it)->TerminalDetect(model.c_str(), (uint8_t *)serial.c_str())) {
                    char portnum[8] = {0};
                    snprintf(portnum, 8, "%d", (*it)->portnum);
                    port = portnum;
                    g_message("New terminal detected @ port %d", (*it)->portnum);
                    feedback.status = "SUCCESS";
                    break;
                }
            }

            if (model == "H3U") {
                g_message("Found a StdModbus device, ip %s", serial.c_str());
                StandardModbusTCP *dev = new StandardModbusTCP(serial.c_str(), 502);
                dev->Connect();
                StdModbusDevices.insert(pair<string, StandardModbusTCP *>(serial.c_str(), dev));
            }

            g_message("Add terminal: [%s, %s, %s, %s]", serial.c_str(), model.c_str(), port.c_str(), baudrate.c_str());
            DS.AddTerminal(serial.c_str(), model.c_str(), port.c_str(), baudrate.c_str());
            set_terminals_table(serial.c_str(), model.c_str(), port.c_str(), atoi(baudrate.c_str()), &Terminal);
        }
    } else if (function == "del-terminal") {
        itr = Json.FindMember("args");
        if (itr != Json.MemberEnd() && itr->value.IsArray()) {
            string serial;
            const Value &args = Json["args"];
            for (unsigned int i = 0; i < args.Size(); i++) {
                const Value &var = args[i];
                if (var.IsObject()) {
                    string N, V;
                    Value::ConstMemberIterator itr_var = var.FindMember("N");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        N = itr_var->value.GetString();
                    }
                    itr_var = var.FindMember("V");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        V = itr_var->value.GetString();
                    }

                    if (N == "tid") serial = V.c_str();
                }
            }

            StdModbusDevices.erase(serial.c_str());

            g_message("Remove terminal: [%s]", serial.c_str());
            DS.RemoveTerminal(serial.c_str());
            remove_terminal(serial.c_str(), &Terminal);
            feedback.status = "SUCCESS";
        }
    } else if (function == "get-terminal") {

    } else if (function == "get-terminals") {

    } else {
        feedback.status = "FAILED";
    }

    stop:
    WS->SendCmdFeedback(feedback);
}

extern PVPoN1 *FC109R;

void terminalcrtl_nsr(const char *payload)
{
    DeviceCmdFeedback feedback;

    Document Json;
    Json.Parse<kParseStopWhenDoneFlag>(payload, strlen(payload));
    if (Json.HasParseError()) {
        return;
    }

    string id;
    string function;

    Value::ConstMemberIterator itr = Json.FindMember("id");
    if (itr != Json.MemberEnd() && itr->value.IsString()) {
        id = itr->value.GetString();
        feedback.id = id;
    }

    itr = Json.FindMember("function");
    if (itr != Json.MemberEnd() && itr->value.IsString()) {
        function = itr->value.GetString();
    }

    if (function == "set-single") {
        itr = Json.FindMember("args");
        if (itr != Json.MemberEnd() && itr->value.IsArray()) {
            string serial = "";
            string model = "";
            int portnum = 0;
            struct FC810Settings settings_fc810;
            struct FC890Settings settings_fc890;
            struct DataSet settings_fc109r;
            settings_fc109r.Data.FC109.Data.power_on = 0xff;
            settings_fc109r.Data.FC109.Data.speed = 0xff;
            settings_fc109r.Data.FC109.Data.mode = 0xff;
            settings_fc109r.Data.FC109.Data.temperature = 0xff;
            settings_fc109r.Data.FC109.Data.key_lock = 0xff;
            const Value &args = Json["args"];
            for (unsigned int i = 0; i < args.Size(); i++) {
                const Value &var = args[i];
                if (var.IsObject()) {
                    string N, V;
                    Value::ConstMemberIterator itr_var = var.FindMember("N");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        N = itr_var->value.GetString();
                    }
                    itr_var = var.FindMember("V");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        V = itr_var->value.GetString();
                    }

                    if (N == "tid") {
                        serial = V.c_str();
                        Terminals::iterator it = Terminal.find(serial);
                        if (it == Terminal.end()) {
                            g_message("Invalid terminal: [%s]", serial.c_str());
                            feedback.status = "FAILED";
                            goto stop;
                        }
                        model = it->second.Model;
                        portnum = atoi(it->second.Port.c_str());
                        if (model == "FC810") {
                            if (portnum < RTUs.size()) {
                                RTUs[portnum]->Lock();
                                RTUs[portnum]->ReadAll("FC810", (const uint8_t *)serial.c_str(), FC810_CMD_READALL_SETTINGS);
                                string framebuf = RTUs[portnum]->Framebuf;
                                msleep(160);
                                RTUs[portnum]->Unlock();
                                if (RTUs[portnum]->CheckStatus()) {
                                    memcpy((void *)&settings_fc810, (void *)&framebuf.c_str()[8], sizeof(settings_fc810));
                                } else {
                                    g_message("Failed reading back from terminal");
                                    feedback.status = "FAILED";
                                    goto stop;
                                }
                            }
                        } else if (model == "FC890") {
                            if (portnum < RTUs.size()) {
                                RTUs[portnum]->Lock();
                                RTUs[portnum]->ReadAll("FC890", (const uint8_t *)serial.c_str(), FC890_CMD_READALL_SETTINGS);
                                string framebuf = RTUs[portnum]->Framebuf;
                                msleep(160);
                                RTUs[portnum]->Unlock();
                                if (RTUs[portnum]->CheckStatus()) {
                                    memcpy((void *)&settings_fc890, (void *)&framebuf.c_str()[8], sizeof(settings_fc890));
                                } else {
                                    g_message("Failed reading back from terminal");
                                    feedback.status = "FAILED";
                                    goto stop;
                                }
                            }
                        }
                        continue;
                    }

                    if (model == "FC810") {
                        uint16_t val = (uint16_t) atoi(V.c_str());
                        if (N == "power_on") {
                            settings_fc810.power_on = val;
                        } else if (N == "setting_mode") {
                            settings_fc810.setting_mode = val;
                        } else if (N == "setting_enable_fanonly") {
                            settings_fc810.setting_enable_fanonly = val;
                        } else if (N == "setting_speed") {
                            settings_fc810.setting_speed = val;
                        } else if (N == "setting_temperature") {
                            settings_fc810.setting_temperature_h = val >> 8;
                            settings_fc810.setting_temperature_l = val & 0x00ff;
                        } else if (N == "setting_key_lock") {
                            settings_fc810.setting_key_lock = val;
                        } else if (N == "setting_op_at_acback") {
                            settings_fc810.setting_op_at_acback = val;
                        }
                    } else if (model == "FC890") {
                        uint16_t val = (uint16_t) atoi(V.c_str());
                    } else if (model == "FC109R") {
                        uint16_t val = (uint16_t) atoi(V.c_str());
                        if (N == "power_on") {
                            settings_fc109r.Data.FC109.Data.power_on = val;
                        } else if (N == "speed") {
                            settings_fc109r.Data.FC109.Data.speed = val;
                        } else if (N == "mode") {
                            settings_fc109r.Data.FC109.Data.mode = val;
                        } else if (N == "temperature") {
                            settings_fc109r.Data.FC109.Data.temperature = val;
                        } else if (N == "key_lock") {
                            settings_fc109r.Data.FC109.Data.key_lock = val;
                        }
                    }
                }
            }

            if (model == "FC810") {
                RTUs[portnum]->Lock();
                int rc = RTUs[portnum]->WriteMulti(model.c_str(), (uint8_t *)serial.c_str(), 0x0302, 8, (uint8_t *)&settings_fc810);
                msleep(160);
                RTUs[portnum]->Unlock();
                if (rc) {
                    Collectors[portnum]->SendImmeSingle(serial.c_str());
                    feedback.status = "SUCCESS";
                } else {
                    feedback.status = "FAILED";
                }
            } else if (model == "FC890") {

            } else if (model == "FC109R") {
                if (!FC109R->FC109R_SetSingle(serial.c_str(), settings_fc109r)) {
                    feedback.status = "SUCCESS";
                } else {
                    feedback.status = "FAILED";
                }
            }
        }
    } else {
        feedback.status = "FAILED";
    }

    stop:
    WS->SendCmdFeedback(feedback);
}

void multicast_nsr(const char *payload)
{
    DeviceCmdFeedback feedback;

    Document Json;
    Json.Parse<kParseStopWhenDoneFlag>(payload, strlen(payload));
    if (Json.HasParseError()) {
        return;
    }

    string id;
    string function;

    Value::ConstMemberIterator itr = Json.FindMember("id");
    if (itr != Json.MemberEnd() && itr->value.IsString()) {
        id = itr->value.GetString();
        feedback.id = id;
    }

    itr = Json.FindMember("function");
    if (itr != Json.MemberEnd() && itr->value.IsString()) {
        function = itr->value.GetString();
    }

    if (function == "set-counter") {
        itr = Json.FindMember("args");
        if (itr != Json.MemberEnd() && itr->value.IsArray()) {
            const Value &args = Json["args"];
            vector<struct commandItem> cmd_list;
            cmd_list.clear();
            for (unsigned int i = 0; i < args.Size(); i++) {
                const Value &var = args[i];
                if (var.IsObject()) {
                    string N, V;
                    Value::ConstMemberIterator itr_var = var.FindMember("N");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        N = itr_var->value.GetString();
                    }
                    itr_var = var.FindMember("V");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        V = itr_var->value.GetString();
                    }

                    if (N == "tid") {
                        prepare_counter_target_list(V, cmd_list);
                    } else {
                        save_conunter_params(N, V, cmd_list);
                    }
                }
            }

            int err_count = 0;

            for (vector<struct commandItem>::iterator it = cmd_list.begin(); it != cmd_list.end(); it++) {
                string model = (*it).model;
                string serial = (*it).tid;
                int portnum = (*it).port;
                if (model == "FC810") {
                    RTUs[portnum]->Lock();
                    int rc = RTUs[portnum]->WriteMulti(model.c_str(), (uint8_t *)serial.c_str(), 16, 9, (uint8_t *)&(((*it).msg)[16]));
                    msleep(160);
                    RTUs[portnum]->Unlock();
                    if (rc) {
                        Collectors[portnum]->SendImmeSingle(serial.c_str());
                    } else {
                        err_count++;
                    }
                } else if (model == "FC890") {
                    RTUs[portnum]->Lock();
                    int rc = RTUs[portnum]->WriteMulti(model.c_str(), (uint8_t *)serial.c_str(), 16, 3, (uint8_t *)&(((*it).msg)[16]));
                    msleep(160);
                    RTUs[portnum]->Unlock();
                    if (rc) {
                        Collectors[portnum]->SendImmeSingle(serial.c_str());
                    } else {
                        err_count++;
                    }
                }
            }

            if (cmd_list.size()) {
                if (err_count) {
                    feedback.status = "FAILED";
                } else {
                    feedback.status = "SUCCESS";
                }
            }
        }
    } else if (function == "set-multi") {
        itr = Json.FindMember("args");
        if (itr != Json.MemberEnd() && itr->value.IsArray()) {
            const Value &args = Json["args"];
            vector<struct commandItem> cmd_list;
            for (unsigned int i = 0; i < args.Size(); i++) {
                const Value &var = args[i];
                if (var.IsObject()) {
                    string N, V;
                    Value::ConstMemberIterator itr_var = var.FindMember("N");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        N = itr_var->value.GetString();
                    }
                    itr_var = var.FindMember("V");
                    if (itr_var != var.MemberEnd() && itr_var->value.IsString()) {
                        V = itr_var->value.GetString();
                    }

                    if (N == "tid") {
                        prepare_target_list(V, cmd_list);
                    } else {
                        save_params(N, V, cmd_list);
                    }
                }
            }

            int err_count = 0;

            vector<string> fc109r_ids;
            struct DataSet fc109r_params;
            fc109r_ids.clear();

            for (vector<struct commandItem>::iterator it = cmd_list.begin(); it != cmd_list.end(); it++) {
                string model = (*it).model;
                string serial = (*it).tid;
                int portnum = (*it).port;
                if (model == "FC810") {
                    RTUs[portnum]->Lock();
                    int rc = RTUs[portnum]->WriteMulti(model.c_str(), (uint8_t *)serial.c_str(), 0x0302, 8, (uint8_t *)&((*it).msg));
                    msleep(160);
                    RTUs[portnum]->Unlock();
                    if (rc) {
                        Collectors[portnum]->SendImmeSingle(serial.c_str());
                    } else {
                        err_count++;
                    }
                } else if (model == "FC890") {

                } else if (model == "FC109R") {
//                    fc109r_ids.push_back(serial.c_str());
//                    fc109r_params = (*it).ds;
                    if (FC109R->FC109R_SetSingle(serial.c_str(), (*it).ds)) err_count++;
                }
            }

            // if (fc109r_ids.size()) err_count = FC109R->FC109R_SetMulti(fc109r_ids, fc109r_params);

            if (cmd_list.size()) {
                if (err_count) {
                    feedback.status = "FAILED";
                } else {
                    feedback.status = "SUCCESS";
                }
            }
        }
    } else {
        feedback.status = "FAILED";
    }

    stop:
    WS->SendCmdFeedback(feedback);
}

extern H3U *H3Us;

void modbus_over_http_nsr(const char *payload)
{
    Document Json;
    Json.Parse<kParseStopWhenDoneFlag>(payload, strlen(payload));
    if (Json.HasParseError()) {
        return;
    }

    string function;
    Value::ConstMemberIterator itr = Json.FindMember("function");
    if (itr != Json.MemberEnd() && itr->value.IsString()) {
        function = itr->value.GetString();
    } else {
        return;
    }

    string slave_id;
    itr = Json.FindMember("slave-id");
    if (itr != Json.MemberEnd() && itr->value.IsString()) {
        slave_id = itr->value.GetString();
    } else {
        return;
    }

    if (function == "interaction") {
        int opcode;
        itr = Json.FindMember("opcode");
        if (itr != Json.MemberEnd() && itr->value.IsInt()) {
            opcode = itr->value.GetInt();
        } else {
            return;
        }

        int addr;
        itr = Json.FindMember("addr");
        if (itr != Json.MemberEnd() && itr->value.IsInt()) {
            addr = itr->value.GetInt();
        } else {
            return;
        }

        int bytes;
        itr = Json.FindMember("bytes");
        if (itr != Json.MemberEnd() && itr->value.IsInt() && itr->value.GetInt() >= 1) {
            bytes = itr->value.GetInt();
        } else {
            return;
        }

        uint8_t *tab_bits = NULL;
        uint16_t *tab_registers = NULL;
        int i32 = 0;

        itr = Json.FindMember("data");
        if (itr != Json.MemberEnd() && itr->value.IsArray()) {
            const Value &args = Json["data"];
            if (opcode == 5 || opcode == 15 || opcode == 6 || opcode == 16) {
                if (args.Size() != bytes) {
                    return;
                }
            }

            switch (opcode) {
            case 5:
            case 6:
                i32 = args[0].GetInt();
                break;
            case 15:
                tab_bits = (uint8_t *) malloc(bytes * sizeof(uint8_t));
                memset(tab_bits, 0, bytes * sizeof(uint8_t));
                for (unsigned int i = 0; i < args.Size(); i++) {
                    const Value &var = args[i];
                    if (var.IsInt()) {
                        tab_bits[i] = var.GetInt();
                    }
                }
                break;
            case 16:
                tab_registers = (uint16_t *) malloc(bytes * sizeof(uint16_t));
                memset(tab_registers, 0, bytes * sizeof(uint16_t));
                for (unsigned int i = 0; i < args.Size(); i++) {
                    const Value &var = args[i];
                    if (var.IsInt()) {
                        tab_registers[i] = var.GetInt();
                    }
                }
                break;
            }
        }

        H3Us->SetCmd(slave_id.c_str(), addr, opcode, bytes, i32, tab_bits, tab_registers);

    } else if (function == "set-attr") {
        DS.RemoveStdModbusAttrs(slave_id.c_str());
        DS.AddStdModbusAttrs(slave_id.c_str(), payload);
        SetModbusAttrs(slave_id.c_str(), payload, &ModbusAttrs);
    }
}
