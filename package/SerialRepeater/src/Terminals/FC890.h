#ifndef FC890_H
#define FC890_H

#include <evloop.h>
#include <map>
#include <queue>

#include <models.h>
#include <rtu.h>
#include <collect.h>

using namespace std;

// Commands: Read Multi
#define FC890_CMD_READALL_STATUS        0
#define FC890_CMD_READALL_COUNTER       1
#define FC890_CMD_READALL_SETTINGS      2

// Commands: Read Single

// Commands: Write Multi

// Commands: Write Single

int FC890_ReadAll(const unsigned char *id, int cmdid, unsigned char *cmdbuf, int buflen);

int FC890_ReadInputRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen);

int FC890_ReadHoldingRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen);

int FC890_WriteMultiRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *value, unsigned char *cmdbuf, int buflen);

int FC890_WriteSingleRegister(const unsigned char *id, unsigned short addr, unsigned char value, unsigned char *cmdbuf, int buflen);

void FC890_ReadMessage(evutil_socket_t fd, short flags, void* args);

void FC890_Update(const char *id, const unsigned char *raw, int type, bool submit, TerminalDataSet &dataset, Document *Json);

#endif // FC890_H
