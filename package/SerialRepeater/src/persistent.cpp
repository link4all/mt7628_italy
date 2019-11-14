#include "persistent.h"

// Create Terminals Table SQL statement
static const char *create_terminals_table = "CREATE TABLE sr_terminals("  \
                                            "serial         CHAR(64) PRIMARY KEY    NOT NULL," \
                                            "port           CHAR(64)                NOT NULL," \
                                            "model          CHAR(64)                NOT NULL," \
                                            "baudrate       CHAR(64)                NOT NULL);";

// Create Rawdata Table SQL statement
static const char *create_rawdata_table =   "CREATE TABLE sr_storage("  \
                                            "serial         CHAR(64) PRIMARY KEY    NOT NULL," \
                                            "create_at      CHAR(64)                NOT NULL," \
                                            "rawdata        BLOB                    NOT NULL);";

// Create Rawdata Table SQL statement
static const char *create_smdattr_table =   "CREATE TABLE sr_smdattrs("  \
                                            "serial         CHAR(64) PRIMARY KEY    NOT NULL," \
                                            "attrs          TEXT                    NOT NULL);";

DataStorage::DataStorage(const char *dbpath_)
{
    dbpath = dbpath_;
    if (sqlite3_open(dbpath_, &db)) db = NULL;
}

DataStorage::~DataStorage()
{
    sqlite3_close(db);
}

static int generic_cb(void *unused, int argc, char **argv, char **col_name)
{
    return 0;
}

static int lookup_terminal_cb(void *user, int argc, char **argv, char **col_name)
{
    return 0;
}

static int list_terminal_cb(void *user, int argc, char **argv, char **col_name)
{
    string serial;
    string model;
    string port;
    string baudrate;

    for (int i = 0; i < argc; i++) {
        if (col_name[i] && argv[i] && !strcmp(col_name[i], "serial")) {
            serial = argv[i];
        } else if (col_name[i] && argv[i] && !strcmp(col_name[i], "port")) {
            port = argv[i];
        } else if (col_name[i] && argv[i] && !strcmp(col_name[i], "model")) {
            model = argv[i];
        } else if (col_name[i] && argv[i] && !strcmp(col_name[i], "baudrate")) {
            baudrate = argv[i];
        }
    }

    Terminals *T = (Terminals *)user;
    set_terminals_table(serial.c_str(), model.c_str(), port.c_str(), atoi(baudrate.c_str()), T);

    return 0;
}

static int list_attrs_cb(void *user, int argc, char **argv, char **col_name)
{
    string serial;
    string attrs;

    for (int i = 0; i < argc; i++) {
        if (col_name[i] && argv[i] && !strcmp(col_name[i], "serial")) {
            serial = argv[i];
        } else if (col_name[i] && argv[i] && !strcmp(col_name[i], "attrs")) {
            attrs = argv[i];
        }
    }

    StdModbusAttrTab *T = (StdModbusAttrTab *)user;
    SetModbusAttrs(serial.c_str(), attrs.c_str(), T);

    return 0;
}

void DataStorage::INITDS()
{
    char *errmsg = NULL;
    int rc = sqlite3_exec(db, create_terminals_table, generic_cb, 0, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Create terminals table error %s", errmsg);
    }
    sqlite3_free(errmsg);

    errmsg = NULL;
    rc = sqlite3_exec(db, create_rawdata_table, generic_cb, 0, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Create rawdata table error %s", errmsg);
    }
    sqlite3_free(errmsg);

    errmsg = NULL;
    rc = sqlite3_exec(db, create_smdattr_table, generic_cb, 0, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Create smdattrs table error %s", errmsg);
    }
    sqlite3_free(errmsg);
}

bool DataStorage::DSCK()
{

}

static const char *insert_terminal = "INSERT INTO sr_terminals (serial, port, model, baudrate) " \
                                     "VALUES ('%s', '%s', '%s', '%s');";

static const char *list_terminals = "SELECT * from sr_terminals";

static const char *query_terminal = "SELECT * from sr_terminals WHERE serial='%s'";

static const char *del_terminal = "DELETE from sr_terminals WHERE serial='%s'";

static const char *insert_attrs = "INSERT INTO sr_smdattrs (serial, attrs) " \
                                     "VALUES ('%s', '%s');";

static const char *list_attrs = "SELECT * from sr_smdattrs";

static const char *query_attrs = "SELECT * from sr_smdattrs WHERE serial='%s'";

static const char *del_attrs = "DELETE from sr_smdattrs WHERE serial='%s'";

int DataStorage::AddTerminal(const char *tid, const char *model, const char *port, const char *baud)
{
    char query[2048] = {0};
    snprintf(query, 2048, insert_terminal, tid, port, model, baud);

    char *errmsg = NULL;
    int rc = sqlite3_exec(db, query, generic_cb, 0, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Insert terminals table error %s", errmsg);
    }
    sqlite3_free(errmsg);

    return rc;
}

int DataStorage::RemoveTerminal(const char *tid)
{
    char query[2048] = {0};
    snprintf(query, 2048, del_terminal, tid);

    char *errmsg = NULL;
    int rc = sqlite3_exec(db, query, generic_cb, 0, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Remove terminal error %s", errmsg);
    }
    sqlite3_free(errmsg);

    return rc;
}

int DataStorage::UpdateTerminalPort(const char *tid, const char *port)
{
    return 0;
}

int DataStorage::GetTerminal(const char *tid, TerminalInfo *r)
{
    char query[2048] = {0};
    snprintf(query, 2048, query_terminal, tid);

    char *errmsg = NULL;
    int rc = sqlite3_exec(db, query, lookup_terminal_cb, r, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Query terminal error %s", errmsg);
    }
    sqlite3_free(errmsg);

    return rc;
}

int DataStorage::GetTerminals(Terminals *r)
{
    char *errmsg = NULL;
    int rc = sqlite3_exec(db, list_terminals, list_terminal_cb, r, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Get terminals error %s", errmsg);
    }
    sqlite3_free(errmsg);

    return rc;
}

int DataStorage::AddStdModbusAttrs(const char *tid, const char *desc)
{
    GString *query = g_string_sized_new(4096);
    g_string_printf(query, insert_attrs, tid, desc);

    char *errmsg = NULL;
    int rc = sqlite3_exec(db, query->str, generic_cb, 0, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Insert attrs table error %s", errmsg);
    }
    sqlite3_free(errmsg);
    g_string_free(query, TRUE);

    return rc;
}

int DataStorage::RemoveStdModbusAttrs(const char *tid)
{
    char query[2048] = {0};
    snprintf(query, 2048, del_attrs, tid);

    char *errmsg = NULL;
    int rc = sqlite3_exec(db, query, generic_cb, 0, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Remove attrs error %s", errmsg);
    }
    sqlite3_free(errmsg);

    return rc;
}

int DataStorage::GetStdModbusAttrs(StdModbusAttrTab *t)
{
    char *errmsg = NULL;
    int rc = sqlite3_exec(db, list_attrs, list_attrs_cb, t, &errmsg);
    if (rc != SQLITE_OK) {
        g_message("Get terminals error %s", errmsg);
    }
    sqlite3_free(errmsg);

    return rc;
}
