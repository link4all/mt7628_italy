#include "HM01.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <rtu.h>

static char ascii_tb[128] = {  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0, 0, 0, 0, 0, 0,
                            10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            10, 11, 12, 13, 14, 15, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                            0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

static void id_convert(const unsigned char *ibuf, unsigned char *obuf, int olen)
{
    if (!ibuf || !obuf) return;

    memset(obuf, 0, olen);
    int index = 0;
    for (int i = 0; i < strlen((char *)ibuf)/2; i++) {
        if (ibuf[i] > 127 || ibuf[i] < 0 || ibuf[i+1] > 127 || ibuf[i+1] < 0 || index >= olen) break;
        char h = ascii_tb[ibuf[i*2]];
        char l = ascii_tb[ibuf[i*2+1]];
        char c = (h << 4) | l;
        obuf[index] = c;
        index++;
    }
}

static uint8_t checksum(uint8_t *buffer, uint16_t buffer_length)
{
    int sum = 0;
    for (int i = 0; i < buffer_length; i++) {
        sum += buffer[i];
    }
    return (uint8_t)(sum & 0xff);
}

static bool Checksum(uint8_t *buffer, uint16_t buffer_length)
{
    uint8_t sum = checksum(buffer, buffer_length-2);
    return (sum == buffer[buffer_length-2]);
}

static const unsigned char HM01CMD_READALL[] =
{
    0xFE, 0xFE, 0x68, 0x20, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x11, 0x11, 0x01, 0x03, 0x90, 0x1F, 0x00, 0xFF, 0x16
};

int HM01_ReadAll(const unsigned char *id, int cmdid, unsigned char *cmdbuf, int buflen)
{
    int cmdlen = sizeof(HM01CMD_READALL);
    int payload_len = cmdlen - 4;

    if (!id || !cmdbuf || buflen < cmdlen) return 0;

    memset(cmdbuf, 0, buflen);
    memcpy(cmdbuf, HM01CMD_READALL, cmdlen);

    unsigned char id_byte[4] = {0};
    id_convert(id, id_byte, 4);
    cmdbuf[4] = id_byte[0];
    cmdbuf[5] = id_byte[1];
    cmdbuf[6] = id_byte[2];
    cmdbuf[7] = id_byte[3];

    unsigned char crc = checksum(&cmdbuf[2], payload_len);
    cmdbuf[cmdlen-2] = crc;

    return cmdlen;
}

int HM01_ReadInputRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen)
{
    return 0;
}

int HM01_ReadHoldingRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen)
{
    return 0;
}

int HM01_WriteMultiRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *value, unsigned char *cmdbuf, int buflen)
{
    return 0;
}

int HM01_WriteSingleRegister(const unsigned char *id, unsigned short addr, unsigned char value, unsigned char *cmdbuf, int buflen)
{
    return 0;
}

void HM01_ReadMessage(evutil_socket_t fd, short flags, void* args)
{
    (void) flags;
    RTU *rtu = (RTU *)args;

    unsigned char buf[20] = {0};
    ssize_t nread = read(fd, buf, 16);
    while ((nread > 0) || ((nread < 0) && (errno == EINTR))) {
        if (nread < 0) {
            nread = read(fd, buf, 16);
            continue;
        } else {
            for (int i = 0; i < nread; i++) {
                switch (rtu->State) {
                case RTU_STATE_IDLE:
                    if (buf[i] == 0x68) {
                        rtu->Framebuf.clear();
                        rtu->Framebuf.push_back(buf[i]);
                    }
                    if (buf[i] == 0x20 && rtu->Framebuf.length() >= 1) {
                        rtu->Framebuf.push_back(buf[i]);
                        rtu->State = RTU_STATE_READ_ADDR;
                    }
                    break;
                case RTU_STATE_READ_ADDR:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= 9) {
                        rtu->State = RTU_STATE_READ_FUNC;
                    }
                    break;
                case RTU_STATE_READ_FUNC:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= 10) {
                        if (buf[i] == 0x81) {
                            rtu->State = RTU_STATE_READ_LEN;
                        }  else {
                            rtu->State = RTU_STATE_ERROR;
                        }
                    }
                    break;
                case RTU_STATE_READ_LEN:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= 11) {
                        rtu->State = RTU_STATE_READ_BYTES;
                    }
                    break;
                case RTU_STATE_READ_BYTES:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= (11 + rtu->Framebuf.c_str()[10])) {
                        rtu->State = RTU_STATE_READ_SUM;
                    }
                    break;
                case RTU_STATE_READ_SUM:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= (13 + rtu->Framebuf.c_str()[10])) {
                        rtu->State = RTU_STATE_IDLE;
                        rtu->CRC = Checksum((uint8_t *)rtu->Framebuf.c_str(), rtu->Framebuf.length());
                        rtu->ClearBusyFlag();
                        if (!rtu->CRC) rtu->CRCErrorCount++;
                    }
                    break;
                case RTU_STATE_ERROR:
                    rtu->Framebuf.push_back(buf[i]);
                    rtu->State = RTU_STATE_IDLE;
                    rtu->ClearBusyFlag();
                    rtu->CRC = false;
                    rtu->ErrorCount++;
                    rtu->CRCErrorCount++;
                    break;
                default:
                    break;
                }
            }
            nread = read(fd, buf, 16);
        }
    }
}

void HM01_Update(const char *id, const unsigned char *raw, bool submit, TerminalDataSet &dataset, Document *Json)
{
    TerminalDataSet::iterator it = dataset.find((const char *)id);
    if (it != dataset.end()) {
        struct DataSet &ds = it->second;
        memcpy((void *)&ds.Data.HM01.Counter, (void *)&raw[14], sizeof(ds.Data.HM01.Counter));
        if (submit) CollectorRenderJson(ds, Json);
    } else {
        struct DataSet ds;
        ds.Model = "HM01";
        ds.Serial = id;
        memcpy((void *)&ds.Data.HM01.Counter, (void *)&raw[14], sizeof(ds.Data.HM01.Counter));
        dataset.insert(pair<string, struct DataSet>(id, ds));
        if (submit) CollectorRenderJson(ds, Json);
    }
}
