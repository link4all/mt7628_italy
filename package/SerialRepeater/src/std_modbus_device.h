#ifndef STD_MODBUS_DEVICE_H
#define STD_MODBUS_DEVICE_H

#include <modbus/modbus.h>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <string.h>
#include <stdlib.h>

using namespace std;

class StandardModbusTCP {
public:
    string device_id;
    string device_ipaddr;
    int device_port;
    modbus_t *ctx;
    uint32_t old_response_to_sec;
    uint32_t old_response_to_usec;
    uint32_t new_response_to_sec;
    uint32_t new_response_to_usec;
    uint32_t old_byte_to_sec;
    uint32_t old_byte_to_usec;

public:
    StandardModbusTCP(const char *ip, int port);

    ~StandardModbusTCP();

    int Connect();

    int Disconnect();

    int ReadCoilBits(int addr, int n, uint8_t *dest);

    int WriteCoilBit(int addr, int val);

    int WriteCoilBits(int addr, int n, uint8_t *val);

    int ReadHoldingRegisters(int addr, int n, uint16_t *dest);

    int WriteHoldingRegister(int addr, int val);

    int WriteHoldingRegisters(int addr, int n, uint16_t *val);

    int ReadInputRegisters(int addr, int n, uint16_t *dest);
};

class StdModbusOperate {
public:
    StandardModbusTCP *dev;
    int i32;
    uint8_t *i8_array;
    uint8_t *o8_array;
    uint16_t *i16_array;
    uint16_t *o16_array;
    int num;
    int addr;
    int opcode;
    int retry;

public:
    int Exec()
    {
        retry++;
        int rc = -1;
        switch (opcode) {
        case 0x01:
            rc = dev->ReadCoilBits(addr, num, o8_array);
            break;
        case 0x04:
            rc = dev->ReadInputRegisters(addr, num, o16_array);
            break;
        case 0x03:
            rc = dev->ReadHoldingRegisters(addr, num, o16_array);
            break;
        case 0x05:
            rc = dev->WriteCoilBit(addr, i32);
            break;
        case 0x0f:
            if (i8_array) rc = dev->WriteCoilBits(addr, num, i8_array);
            break;
        case 0x06:
            rc = dev->WriteHoldingRegister(addr, i32);
            break;
        case 0x10:
            if (i16_array) rc = dev->WriteHoldingRegisters(addr, num, i16_array);
            break;
        default:
            break;
        }
        return rc;
    }

    StdModbusOperate(StandardModbusTCP *dev_, int op_, int addr_, int num_, int i32_, uint8_t *i8_array_, uint16_t *i16_array_)
    {
        dev = dev_;
        opcode = op_;
        addr = addr_;
        num = num_;
        i32 = i32_;
        i8_array = 0;
        i16_array = 0;
        retry = 0;

        if (i8_array_) {
            i8_array = i8_array_;
        }

        if (i16_array_) {
            i16_array = i16_array_;
        }

        o8_array = new uint8_t[num];
        o16_array = new uint16_t[num];
    }

    ~StdModbusOperate()
    {
        delete o8_array;
        delete o16_array;

        if (i8_array) {
            free(i8_array);
        }

        if (i16_array) {
            free(i16_array);
        }
    }
};

struct StdModbusReadCmd {
    int opcode;
    int addr;
    int bytes;
};
typedef vector<struct StdModbusReadCmd> StdModbusCmdList;
typedef map<string, StdModbusCmdList>   StdModbusAttrTab;

void SetModbusAttrs(const char *id, const char *desc, StdModbusAttrTab *t);

#endif // STD_MODBUS_DEVICE_H
