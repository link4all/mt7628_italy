#include "mqtt.h"

#include <string.h>
#include <stdlib.h>

#include <delay.hpp>

static void message_cb(struct mosquitto *ctx, void *args, const struct mosquitto_message *message)
{
    MQTTClient *mqtt = (MQTTClient *)args;
    if (message->payloadlen) {
        for (vector<struct MQTTTopic>::iterator it = mqtt->topics.begin(); it != mqtt->topics.end(); it++) {
            bool is_matched;
            if (mosquitto_topic_matches_sub((*it).Topic.c_str(), message->topic, &is_matched)) continue;
            if (is_matched) {
                char *payload = (char *) calloc(1, message->payloadlen+1);
                memcpy(payload, message->payload, message->payloadlen);
                g_message("Recieved message on %s, content: %s", message->topic, payload);
                if ((*it).NSR) (*it).NSR(payload);
                free(payload);
                return;
            }
        }
    }
}

static void connect_cb(struct mosquitto *ctx, void *args, int32_t status)
{
    MQTTClient *mqtt = (MQTTClient *)args;
    if (!status) {
        for (vector<struct MQTTTopic>::iterator it = mqtt->topics.begin(); it != mqtt->topics.end(); it++) {
            mosquitto_subscribe(ctx, NULL, (*it).Topic.c_str(), (*it).QoS);
            g_message("Subscribe %s", (*it).Topic.c_str());
        }
    }
}

static void *run_client(void *args)
{
    MQTTClient *mqtt = (MQTTClient *)args;
    mqtt->Run();
}

MQTTClient::MQTTClient(const char *broker_addr_, int port_, int keepalive_, const char *client_id_, const char *username_, const char *password_,
                       bool enable_tls_, const char *ca_crt_, const char *ca_path_, const char *client_crt_, const char *client_key_)
{
    ctx = NULL;
    mosquitto_lib_init();
    ctx = mosquitto_new(client_id_, 0, this);
    if (!ctx) {
        mosquitto_lib_cleanup();
        return;
    }

    if (mosquitto_username_pw_set(ctx, username_, password_)) {
        mosquitto_lib_cleanup();
        return;
    }

    if (enable_tls_) {
        if (mosquitto_tls_set(ctx, ca_crt_, ca_path_, client_crt_, client_key_, NULL)) {
            mosquitto_lib_cleanup();
            return;
        }
    }

    mosquitto_threaded_set(ctx, 1);

    broker_addr = broker_addr_;
    port = port_;
    keepalive = keepalive_;
    client_id = client_id_;
    username = username_;
    password = password_;
    enable_tls = enable_tls_;
    ca_crt = ca_crt_;
    ca_path = ca_path_;
    client_crt = client_crt_;
    client_key = client_key_;
}

MQTTClient::~MQTTClient()
{
    Stop();
    mosquitto_destroy(ctx);
    mosquitto_lib_cleanup();
}

void MQTTClient::Run()
{
    mosquitto_message_callback_set(ctx, message_cb);
    mosquitto_connect_callback_set(ctx, connect_cb);

    int rc = 0;
    for (; rc = mosquitto_connect(ctx, broker_addr.c_str(), port, keepalive); ssleep(10));
    g_message("Connect to MQTT Broker Server: %d", rc);

    while (1) {
        mosquitto_loop_forever(ctx, 60000, 1);
        g_message("MQTT message-loop break");
    }

    // for (; mosquitto_loop_forever(ctx, 60000, 1); ssleep(1));
}

void MQTTClient::Start()
{
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    pthread_create(&threadid, &attr, run_client, (void *)this);
}

void MQTTClient::Stop()
{
    if (ctx) {
        mosquitto_loop_stop(ctx, 1);
        mosquitto_disconnect(ctx);
    }
}

void MQTTClient::Subscribe(const char *topic, int qos, void (*NSR)(const char *))
{
    struct MQTTTopic t;
    t.Topic = topic;
    t.QoS = qos;
    t.NSR = NSR;
    topics.push_back(t);
}

int MQTTClient::Publish(const char *topic, const char *message, long len)
{
    return mosquitto_publish(ctx, NULL, topic, len, message, 1, 0);
}
