#ifndef WSAPI_H
#define WSAPI_H

#include <Utils/http.h>
#include <rapidjson/document.h>

#include <models.h>

using namespace std;

class WSAPI
{
public:
    WSAPI();
    ~WSAPI();

public:
    int Register(DeviceBase &i);
    int SendData(Document *d);
    int SendStatus(DeviceStatus &i);
    int SendPing(DevicePing &i);
    int SendCmdFeedback(DeviceCmdFeedback &i);
    int RequestCert();

private:
    int send_request(HttpRequest &request, Document *Json, const char *api, const char *method);
    void renew_key();
    void lock() { pthread_mutex_lock(&ilock); }
    void unlock() { pthread_mutex_unlock(&ilock); }

private:
    HttpClient  *https;
    string      key;
    string      secret;
    pthread_mutex_t ilock;
};

#endif // WSAPI_H
