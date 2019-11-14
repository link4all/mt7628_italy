#include "FC810.h"

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>

#include <rtu.h>

static const unsigned char FC810CMD_READALL[][13] =
{
    {0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00},
    {0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x03, 0x00, 0x00, 0x00, 0x1A, 0x00, 0x00},
    {0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x03, 0x02, 0x00, 0x00, 0x04, 0x00, 0x00},
    {0x0D, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x02, 0x00, 0x0A, 0x00, 0x00}
};

static bool Checksum(unsigned char *data, long datalen)
{
    unsigned short crc = RTU_CRC16(data, datalen-2);
    return ((data[datalen-2] << 8) | (data[datalen-1]) == crc);
}

int FC810_ReadAll(const unsigned char *id, int cmdid, unsigned char *cmdbuf, int buflen)
{
    int cmdlen = sizeof(FC810CMD_READALL[cmdid]);
    int payload_len = cmdlen - 2;

    if (!id || !cmdbuf || buflen < cmdlen) return 0;

    memset(cmdbuf, 0, buflen);
    memcpy(cmdbuf, FC810CMD_READALL[cmdid], cmdlen);

    unsigned char id_byte[4] = {0};
    RTU_ID_Convert(id, id_byte, 4);
    memcpy(&cmdbuf[2], id_byte, 4);

    unsigned short crc = RTU_CRC16(cmdbuf, payload_len);
    cmdbuf[payload_len] = crc >> 8;
    cmdbuf[payload_len+1] = crc & 0x00ff;

    return cmdlen;
}

int FC810_ReadInputRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen)
{
    if (!id || !cmdbuf || buflen < 64) return 0;

    int cmdlen = 0;
    memset(cmdbuf, 0, buflen);

    cmdbuf[0] = 0x0D;
    cmdbuf[1] = 0x0A;
    cmdlen += 2;

    unsigned char id_byte[4] = {0};
    RTU_ID_Convert(id, id_byte, 4);
    memcpy(&cmdbuf[cmdlen], id_byte, 4);
    cmdlen += 4;

    cmdbuf[cmdlen] = 0x04;
    cmdlen += 1;

    cmdbuf[cmdlen] = addr >> 8;
    cmdbuf[cmdlen+1] = addr & 0x00ff;
    cmdlen += 2;

    cmdbuf[cmdlen] = quantity >> 8;
    cmdbuf[cmdlen+1] = quantity & 0x00ff;
    cmdlen += 2;

    unsigned short crc = RTU_CRC16(cmdbuf, cmdlen);
    cmdbuf[cmdlen] = crc >> 8;
    cmdbuf[cmdlen+1] = crc & 0x00ff;
    cmdlen += 2;

    return cmdlen;
}

int FC810_ReadHoldingRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen)
{
    if (!id || !cmdbuf || buflen < 64) return 0;

    int cmdlen = 0;
    memset(cmdbuf, 0, buflen);

    cmdbuf[0] = 0x0D;
    cmdbuf[1] = 0x0A;
    cmdlen += 2;

    unsigned char id_byte[4] = {0};
    RTU_ID_Convert(id, id_byte, 4);
    memcpy(&cmdbuf[cmdlen], id_byte, 4);
    cmdlen += 4;

    cmdbuf[cmdlen] = 0x03;
    cmdlen += 1;

    cmdbuf[cmdlen] = addr >> 8;
    cmdbuf[cmdlen+1] = addr & 0x00ff;
    cmdlen += 2;

    cmdbuf[cmdlen] = quantity >> 8;
    cmdbuf[cmdlen+1] = quantity & 0x00ff;
    cmdlen += 2;

    unsigned short crc = RTU_CRC16(cmdbuf, cmdlen);
    cmdbuf[cmdlen] = crc >> 8;
    cmdbuf[cmdlen+1] = crc & 0x00ff;
    cmdlen += 2;

    return cmdlen;
}

int FC810_WriteMultiRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *value, unsigned char *cmdbuf, int buflen)
{
    if (!id || !cmdbuf || buflen < 64) return 0;

    int cmdlen = 0;
    memset(cmdbuf, 0, buflen);

    cmdbuf[0] = 0x0D;
    cmdbuf[1] = 0x0A;
    cmdlen += 2;

    unsigned char id_byte[4] = {0};
    RTU_ID_Convert(id, id_byte, 4);
    memcpy(&cmdbuf[cmdlen], id_byte, 4);
    cmdlen += 4;

    cmdbuf[cmdlen] = 0x10;
    cmdlen += 1;

    cmdbuf[cmdlen] = addr >> 8;
    cmdbuf[cmdlen+1] = addr & 0x00ff;
    cmdlen += 2;

    cmdbuf[cmdlen] = quantity >> 8;
    cmdbuf[cmdlen+1] = quantity & 0x00ff;
    cmdlen += 2;

    cmdbuf[cmdlen] = quantity;
    cmdlen += 1;

    memcpy(&cmdbuf[cmdlen], value, quantity);
    cmdlen += quantity;

    unsigned short crc = RTU_CRC16(cmdbuf, cmdlen);
    cmdbuf[cmdlen] = crc >> 8;
    cmdbuf[cmdlen+1] = crc & 0x00ff;
    cmdlen += 2;

    return cmdlen;
}

int FC810_WriteSingleRegister(const unsigned char *id, unsigned short addr, unsigned char value, unsigned char *cmdbuf, int buflen)
{
    if (!id || !cmdbuf || buflen < 64) return 0;

    int cmdlen = 0;
    memset(cmdbuf, 0, buflen);

    cmdbuf[0] = 0x0D;
    cmdbuf[1] = 0x0A;
    cmdlen += 2;

    unsigned char id_byte[4] = {0};
    RTU_ID_Convert(id, id_byte, 4);
    memcpy(&cmdbuf[cmdlen], id_byte, 4);
    cmdlen += 4;

    cmdbuf[cmdlen] = 0x06;
    cmdlen += 1;

    cmdbuf[cmdlen] = addr >> 8;
    cmdbuf[cmdlen+1] = addr & 0x00ff;
    cmdlen += 2;

    cmdbuf[cmdlen] = 0x01;
    cmdbuf[cmdlen+1] = value;
    cmdlen += 2;

    unsigned short crc = RTU_CRC16(cmdbuf, cmdlen);
    cmdbuf[cmdlen] = crc >> 8;
    cmdbuf[cmdlen+1] = crc & 0x00ff;
    cmdlen += 2;

    return cmdlen;
}

void FC810_ReadMessage(evutil_socket_t fd, short flags, void* args)
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
                    if (buf[i] == 0x0D) {
                        rtu->Framebuf.clear();
                        rtu->Framebuf.push_back(buf[i]);
                    }
                    if (buf[i] == 0x0A && rtu->Framebuf.length() >= 1) {
                        rtu->Framebuf.push_back(buf[i]);
                        rtu->State = RTU_STATE_READ_ADDR;
                    }
                    break;
                case RTU_STATE_READ_ADDR:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= 6) {
                        rtu->State = RTU_STATE_READ_FUNC;
                    }
                    break;
                case RTU_STATE_READ_FUNC:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= 7) {
                        if (buf[i] == 0x03 || buf[i] == 0x04) {
                            rtu->State = RTU_STATE_READ_LEN;
                        } else if (buf[i] == 0x06) {
                            rtu->State = RTU_STATE_READ_06_RESPONSE;
                        } else if (buf[i] == 0x10) {
                            rtu->State = RTU_STATE_READ_10_RESPONSE;
                        } else {
                            rtu->State = RTU_STATE_ERROR;
                        }
                    }
                    break;
                case RTU_STATE_READ_06_RESPONSE:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= 13) {
                        rtu->State = RTU_STATE_IDLE;
                        rtu->CRC = Checksum((unsigned char *)rtu->Framebuf.c_str(), rtu->Framebuf.length());
                        rtu->ClearBusyFlag();
                        if (!rtu->CRC) rtu->CRCErrorCount++;
                    }
                    break;
                case RTU_STATE_READ_10_RESPONSE:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= 13) {
                        rtu->State = RTU_STATE_IDLE;
                        rtu->CRC = Checksum((unsigned char *)rtu->Framebuf.c_str(), rtu->Framebuf.length());
                        rtu->ClearBusyFlag();
                        if (!rtu->CRC) rtu->CRCErrorCount++;
                    }
                    break;
                case RTU_STATE_READ_LEN:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= 8) {
                        rtu->State = RTU_STATE_READ_BYTES;
                    }
                    break;
                case RTU_STATE_READ_BYTES:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= (8 + rtu->Framebuf.c_str()[7])) {
                        rtu->State = RTU_STATE_READ_SUM;
                    }
                    break;
                case RTU_STATE_READ_SUM:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= (10 + rtu->Framebuf.c_str()[7])) {
                        rtu->State = RTU_STATE_IDLE;
                        rtu->CRC = Checksum((unsigned char *)rtu->Framebuf.c_str(), rtu->Framebuf.length());
                        rtu->ClearBusyFlag();
                        if (!rtu->CRC) rtu->CRCErrorCount++;
                    }
                    break;
                case RTU_STATE_ERROR:
                    rtu->Framebuf.push_back(buf[i]);
                    if (rtu->Framebuf.length() >= 10) {
                        rtu->State = RTU_STATE_IDLE;
                        rtu->CRC = Checksum((unsigned char *)rtu->Framebuf.c_str(), rtu->Framebuf.length());
                        rtu->ClearBusyFlag();
                        rtu->ErrorCount++;
                        if (!rtu->CRC) rtu->CRCErrorCount++;
                    }
                    break;
                default:
                    break;
                }
            }
            nread = read(fd, buf, 16);
        }
    }
}

void FC810_Update(const char *id, const unsigned char *raw, int type, bool submit, TerminalDataSet &dataset, Document *Json)
{
    TerminalDataSet::iterator it = dataset.find((const char *)id);
    if (it != dataset.end()) {
        struct DataSet &ds = it->second;
        switch (type) {
        case FC810_CMD_READALL_STATUS:
            memcpy((void *)&ds.Data.FC810.Status, (void *)&raw[8], sizeof(ds.Data.FC810.Status));
            break;
        case FC810_CMD_READALL_COUNTER:
            memcpy((void *)&ds.Data.FC810.Counter, (void *)&raw[8], sizeof(ds.Data.FC810.Counter));
            break;
        case FC810_CMD_READALL_LOCATION:
            memcpy((void *)&ds.Data.FC810.Location, (void *)&raw[8], sizeof(ds.Data.FC810.Location));
            break;
        case FC810_CMD_READALL_SETTINGS:
            memcpy((void *)&ds.Data.FC810.Settings, (void *)&raw[8], sizeof(ds.Data.FC810.Settings));
            break;
        }
        if (submit) CollectorRenderJson(ds, Json);
    } else {
        struct DataSet ds;
        ds.Model = "FC810";
        ds.Serial = id;
        switch (type) {
        case FC810_CMD_READALL_STATUS:
            memcpy((void *)&ds.Data.FC810.Status, (void *)&raw[8], sizeof(ds.Data.FC810.Status));
            break;
        case FC810_CMD_READALL_COUNTER:
            memcpy((void *)&ds.Data.FC810.Counter, (void *)&raw[8], sizeof(ds.Data.FC810.Counter));
            break;
        case FC810_CMD_READALL_LOCATION:
            memcpy((void *)&ds.Data.FC810.Location, (void *)&raw[8], sizeof(ds.Data.FC810.Location));
            break;
        case FC810_CMD_READALL_SETTINGS:
            memcpy((void *)&ds.Data.FC810.Settings, (void *)&raw[8], sizeof(ds.Data.FC810.Settings));
            break;
        }
        if (submit) CollectorRenderJson(ds, Json);
        dataset.insert(pair<string, struct DataSet>(id, ds));
    }
}
