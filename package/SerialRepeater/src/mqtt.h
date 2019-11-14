#ifndef MQTT_H
#define MQTT_H

#include <string>
#include <deque>
#include <vector>
#include <map>
#include <pthread.h>

#include <glib-2.0/glib.h>
#include <mosquitto.h>

#include <evloop.h>

using namespace std;

struct MQTTTopic
{
    string Topic;
    int QoS;
    void (*NSR)(const char *);
};

class MQTTClient
{
private:
    pthread_t           threadid;
    struct mosquitto    *ctx;
    string              broker_addr;
    int                 port;
    int                 keepalive;
    string              client_id;
    string              username;
    string              password;
    string              ca_crt;
    string              ca_path;
    string              client_crt;
    string              client_key;
    bool                enable_tls;

public:
    vector<struct MQTTTopic>      topics;

public:
    MQTTClient(const char *broker_addr_, int port_, int keepalive_, const char *client_id_, const char *username_, const char *password_,
               bool enable_tls_, const char *ca_crt_, const char *ca_path_, const char *client_crt_, const char *client_key_);
    ~MQTTClient();

    void Run();
    void Start();
    void Stop();

    void Subscribe(const char *topic, int qos, void (*NSR)(const char *));
    int Publish(const char *topic, const char *message, long len);
};

#endif // MQTT_H
