#ifndef RTU_H
#define RTU_H

#include <string>
#include <deque>
#include <vector>
#include <map>
#include <pthread.h>

#include <glib-2.0/glib.h>

#include <evloop.h>

using namespace std;

typedef map<string, struct TerminalInfo> Terminals;
typedef deque<GString *>                 RTUDATA;

#define RTU_STATE_IDLE                      0
#define RTU_STATE_READ_ADDR                 1
#define RTU_STATE_READ_FUNC                 2
#define RTU_STATE_READ_06_RESPONSE          3
#define RTU_STATE_READ_10_RESPONSE          4
#define RTU_STATE_READ_LEN                  5
#define RTU_STATE_READ_BYTES                6
#define RTU_STATE_READ_SUM                  7
#define RTU_STATE_FINISH                    8
#define RTU_STATE_ERROR                     9

class RTU
{
private:
    int                 fd;
    int                 rate;
    EventLoop           *ev;
    pthread_t           tid;
    pthread_mutex_t     qlock;
    uint64_t            recv_worker;
    long                recv_start;
    bool                recv_busy;

public:
    const char          *port;
    int                 portnum;
    string              Framebuf;
    int                 State;
    char                Error;
    char                Exception;
    bool                CRC;
    uint64_t            CRCErrorCount;
    uint64_t            RXTimeoutCount;
    uint64_t            ErrorCount;

public:
    RTU(const char *portname, int portnum_);
    ~RTU();

    int ResetPort(int baud);
    int ResetPortE(int baud);
    int ReadAll(const char *model, const unsigned char *id, int index);
    int ReadInputReg(const char *model, const unsigned char *id, short addr, short quantity);
    int ReadHoldingReg(const char *model, const unsigned char *id, short addr, short quantity);
    int WriteSingle(const char *model, const unsigned char *id, short addr, unsigned char value);
    int WriteMulti(const char *model, const unsigned char *id, unsigned short addr, unsigned short quantity, unsigned char *value);

    bool CheckStatus() { return ((!Error) && (!Exception) && CRC); }
    bool CheckPortnum(int portnum_) { return portnum == portnum_; }
    bool TerminalDetect(const char *model, const unsigned char *id);
    bool Timeout();
    bool Busy();
    void ClearBusyFlag();

    void Start();
    void Stop();
    void Dispatch();

    void Lock() { pthread_mutex_lock(&qlock); }
    void Unlock() { pthread_mutex_unlock(&qlock); }
};

void RTU_ID_Convert(const unsigned char *ibuf, unsigned char *obuf, int olen);

unsigned short RTU_CRC16(unsigned char *bytes, unsigned short len);

void set_terminals_table(const char *serial, const char *model, const char *port, int baud, Terminals *T);

void remove_terminal(const char *serial, Terminals *T);

#endif // RTU_H
