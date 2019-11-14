#ifndef FC810_H
#define FC810_H

#include <evloop.h>
#include <map>
#include <queue>

#include <models.h>
#include <rtu.h>
#include <collect.h>

using namespace std;

// Commands: Read Multi
#define FC810_CMD_READALL_STATUS        0
#define FC810_CMD_READALL_COUNTER       1
#define FC810_CMD_READALL_LOCATION      2
#define FC810_CMD_READALL_SETTINGS      3

// Commands: Read Single

// Commands: Write Multi

// Commands: Write Single

int FC810_ReadAll(const unsigned char *id, int cmdid, unsigned char *cmdbuf, int buflen);

int FC810_ReadInputRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen);

int FC810_ReadHoldingRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen);

int FC810_WriteMultiRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *value, unsigned char *cmdbuf, int buflen);

int FC810_WriteSingleRegister(const unsigned char *id, unsigned short addr, unsigned char value, unsigned char *cmdbuf, int buflen);

void FC810_ReadMessage(evutil_socket_t fd, short flags, void* args);

void FC810_Update(const char *id, const unsigned char *raw, int type, bool submit, TerminalDataSet &dataset, Document *Json);

#endif // FC810_H
