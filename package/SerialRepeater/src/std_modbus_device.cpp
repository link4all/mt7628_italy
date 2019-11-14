#include "std_modbus_device.h"

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "models.h"

using namespace rapidjson;

StandardModbusTCP::StandardModbusTCP(const char *ip, int port)
{
    device_id = ip;
    device_ipaddr = ip;
    device_port = port;
    ctx = NULL;
}

StandardModbusTCP::~StandardModbusTCP()
{
    Disconnect();
}

int StandardModbusTCP::Connect()
{
    ctx = modbus_new_tcp(device_ipaddr.c_str(), device_port);
    if (ctx) {
        modbus_set_debug(ctx, TRUE);
        modbus_set_error_recovery(ctx, (modbus_error_recovery_mode)(MODBUS_ERROR_RECOVERY_LINK|MODBUS_ERROR_RECOVERY_PROTOCOL));
        modbus_get_response_timeout(ctx, &old_response_to_sec, &old_response_to_usec);
        if (modbus_connect(ctx) == -1) {
            modbus_free(ctx);
            ctx = NULL;
            return -1;
        }
        modbus_get_response_timeout(ctx, &new_response_to_sec, &new_response_to_usec);
        return 0;
    }

    return -1;
}

int StandardModbusTCP::Disconnect()
{
    if (ctx) {
        modbus_close(ctx);
        modbus_free(ctx);
        ctx = NULL;
    }

    return 0;
}

int StandardModbusTCP::ReadCoilBits(int addr, int n, uint8_t *dest)
{
    if (ctx) {
        int rc = modbus_read_bits(ctx, addr, n, dest);
        return rc;
    } else {
        Connect();
        return -1;
    }
}

int StandardModbusTCP::WriteCoilBit(int addr, int val)
{
    if (ctx) {
        int rc = modbus_write_bit(ctx, addr, val);
        return rc;
    } else {
        Connect();
        return -1;
    }
}

int StandardModbusTCP::WriteCoilBits(int addr, int n, uint8_t *val)
{
    uint8_t tab_value[n];
    modbus_set_bits_from_bytes(tab_value, 0, n, val);

    if (ctx) {
        int rc = modbus_write_bits(ctx, addr, n, tab_value);
        return rc;
    } else {
        Connect();
        return -1;
    }
}

int StandardModbusTCP::ReadHoldingRegisters(int addr, int n, uint16_t *dest)
{
    if (ctx) {
        int rc = modbus_read_registers(ctx, addr, n, dest);
        return rc;
    } else {
        Connect();
        return -1;
    }
}

int StandardModbusTCP::WriteHoldingRegister(int addr, int val)
{
    if (ctx) {
        int rc = modbus_write_register(ctx, addr, val);
        return rc;
    } else {
        Connect();
        return -1;
    }
}

int StandardModbusTCP::WriteHoldingRegisters(int addr, int n, uint16_t *val)
{
    if (ctx) {
        int rc = modbus_write_registers(ctx, addr, n, val);
        return rc;
    } else {
        Connect();
        return -1;
    }
}

int StandardModbusTCP::ReadInputRegisters(int addr, int n, uint16_t *dest)
{
    if (ctx) {
        int rc = modbus_read_input_registers(ctx, addr, n, dest);
        return rc;
    } else {
        Connect();
        return -1;
    }
}

void SetModbusAttrs(const char *id, const char *desc, StdModbusAttrTab *t)
{
    Document Json;
    Json.Parse<kParseStopWhenDoneFlag>(desc, strlen(desc));
    if (Json.HasParseError()) {
        return;
    }

    t->erase(id);

    set<int> addr_coil;
    set<int> addr_input;
    set<int> addr_hold;

    addr_coil.clear();
    addr_input.clear();
    addr_hold.clear();

    Value::ConstMemberIterator itr = Json.FindMember("attrs");
    if (itr != Json.MemberEnd() && itr->value.IsArray()) {
        const Value &attrs = Json["attrs"];
        for (unsigned int i = 0; i < attrs.Size(); i++) {
            const Value &var = attrs[i];
            if (var.IsObject()) {
                int opcode = 0;
                Value::ConstMemberIterator itr_var = var.FindMember("opcode");
                if (itr_var != var.MemberEnd() && itr_var->value.IsInt()) {
                    opcode = itr_var->value.GetInt();
                }

                itr_var = var.FindMember("addr");
                if (itr_var != var.MemberEnd() && itr_var->value.IsArray()) {
                    const Value &addrs = var["addr"];
                    for (unsigned int ii = 0; ii < addrs.Size(); ii++) {
                        const Value &addr = addrs[ii];
                        if (addr.IsInt()) {
                            if (opcode == 1) {
                                addr_coil.insert(addr.GetInt());
                            } else if (opcode == 3) {
                                addr_input.insert(addr.GetInt());
                            } else if (opcode == 4) {
                                addr_hold.insert(addr.GetInt());
                            }
                        }
                    }
                }
            }
        }
    }

    int last_addr = 0;
    int first_addr = 0;
    int bytes = 0;

    StdModbusCmdList cmds;

    int count = 0;
    for (set<int>::iterator it = addr_coil.begin(); it != addr_coil.end(); it++) {
        int addr = *it;
        if (it == addr_coil.begin()) {
            last_addr = addr;
            first_addr = addr;
            bytes = 1;
        } else if (addr == last_addr + 1) {
            last_addr = addr;
            bytes++;
        } else if (addr > last_addr + 1) {
            struct StdModbusReadCmd cmd;
            cmd.addr = first_addr;
            cmd.bytes = bytes;
            cmd.opcode = 1;
            cmds.push_back(cmd);

            last_addr = addr;
            first_addr = addr;
            bytes = 1;
        }

        count++;
        if (count == addr_coil.size()) {
            struct StdModbusReadCmd cmd;
            cmd.addr = first_addr;
            cmd.bytes = bytes;
            cmd.opcode = 1;
            cmds.push_back(cmd);
        }
    }

    count = 0;
    for (set<int>::iterator it = addr_input.begin(); it != addr_input.end(); it++) {
        int addr = *it;
        if (it == addr_input.begin()) {
            last_addr = addr;
            first_addr = addr;
            bytes = 1;
        } else if (addr == last_addr + 1) {
            last_addr = addr;
            bytes++;
        } else if (addr > last_addr + 1) {
            struct StdModbusReadCmd cmd;
            cmd.addr = first_addr;
            cmd.bytes = bytes;
            cmd.opcode = 3;
            cmds.push_back(cmd);

            last_addr = addr;
            first_addr = addr;
            bytes = 1;
        }

        count++;
        if (count == addr_input.size()) {
            struct StdModbusReadCmd cmd;
            cmd.addr = first_addr;
            cmd.bytes = bytes;
            cmd.opcode = 3;
            cmds.push_back(cmd);
        }
    }

    count = 0;
    for (set<int>::iterator it = addr_hold.begin(); it != addr_hold.end(); it++) {
        int addr = *it;
        if (it == addr_hold.begin()) {
            last_addr = addr;
            first_addr = addr;
            bytes = 1;
        } else if (addr == last_addr + 1) {
            last_addr = addr;
            bytes++;
        } else if (addr > last_addr + 1) {
            struct StdModbusReadCmd cmd;
            cmd.addr = first_addr;
            cmd.bytes = bytes;
            cmd.opcode = 4;
            cmds.push_back(cmd);

            last_addr = addr;
            first_addr = addr;
            bytes = 1;
        }

        count++;
        if (count == addr_hold.size()) {
            struct StdModbusReadCmd cmd;
            cmd.addr = first_addr;
            cmd.bytes = bytes;
            cmd.opcode = 4;
            cmds.push_back(cmd);
        }
    }

    g_message("CMDQ: @%s", id);
    for (vector<StdModbusReadCmd>::iterator it = cmds.begin(); it != cmds.end(); it++) {
        struct StdModbusReadCmd &cmd = *it;
        g_message("-> opcode=%d, addr=%d, bytes=%d", cmd.opcode, cmd.addr, cmd.bytes);
    }

    t->insert(pair<string, StdModbusCmdList>(id, cmds));
}
