#ifndef HM01_H
#define HM01_H

#include <evloop.h>
#include <map>
#include <queue>

#include <models.h>
#include <rtu.h>
#include <collect.h>

using namespace std;

// Commands: Read Multi
#define HM01_CMD_READALL_STATUS        0
#define HM01_CMD_READALL_COUNTER       1
#define HM01_CMD_READALL_LOCATION      2
#define HM01_CMD_READALL_SETTINGS      3

// Commands: Read Single

// Commands: Write Multi

// Commands: Write Single

int HM01_ReadAll(const unsigned char *id, int cmdid, unsigned char *cmdbuf, int buflen);

int HM01_ReadInputRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen);

int HM01_ReadHoldingRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *cmdbuf, int buflen);

int HM01_WriteMultiRegister(const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *value, unsigned char *cmdbuf, int buflen);

int HM01_WriteSingleRegister(const unsigned char *id, unsigned short addr, unsigned char value, unsigned char *cmdbuf, int buflen);

void HM01_ReadMessage(evutil_socket_t fd, short flags, void* args);

void HM01_Update(const char *id, const unsigned char *raw, bool submit, TerminalDataSet &dataset, Document *Json);

#endif // HM01_H
