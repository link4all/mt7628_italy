#ifndef PERSISTENT_H
#define PERSISTENT_H

#include <map>

#include <sqlite3.h>

#include <models.h>
#include <std_modbus_device.h>

using namespace std;

class DataStorage {
private:
    sqlite3     *db;
    const char  *dbpath;

public:

private:

public:
    DataStorage(const char *dbpath_);
    ~DataStorage();

    void INITDS();
    bool DSCK();

    // Terminals
    int AddTerminal(const char *tid, const char *model, const char *port, const char *baud);
    int RemoveTerminal(const char *tid);
    int UpdateTerminalPort(const char *tid, const char *port);
    int GetTerminal(const char *tid, struct TerminalInfo *r);
    int GetTerminals(Terminals *r);

    // Attrs Table
    int AddStdModbusAttrs(const char *tid, const char *desc);
    int RemoveStdModbusAttrs(const char *tid);
    int GetStdModbusAttrs(StdModbusAttrTab *t);
};

#endif // PERSISTENT_H
