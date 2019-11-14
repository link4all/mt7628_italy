#include "wsapi.h"

#include <stdio.h>
#include <string>
#include <vector>
#include <deque>
#include <map>
#include <set>
#include <sys/time.h>

#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <baseinfo.h>
#include <profiles.h>

static inline long timestamp()
{
    struct timeval tv = {0};
    gettimeofday(&tv, NULL);
    long nonce = (long long) tv.tv_sec;
    return nonce;
}

static const gchar hex_digits[] = "0123456789abcdef";
static gchar *digest_to_string(uint8_t *digest, uint32_t digest_len)
{
    gint len = digest_len * 2;
    gint i;
    gchar *retval;

    retval = g_new(gchar, len + 1);

    for (i = 0; i < digest_len; i++) {
        uint8_t byte = digest[i];

        retval[2 * i] = hex_digits[byte >> 4];
        retval[2 * i + 1] = hex_digits[byte & 0xf];
    }

    retval[len] = 0;

    return retval;
}

static inline int signature(const char *key, const char *secret, const char *method, const char *path, HttpRequest &request)
{
    if (!key || !secret || !method || !path) return -1;

    GString *signature_string = g_string_sized_new(4096);

    long ts = timestamp() + 5;
    g_string_append(signature_string, DeviceSerialNumber.c_str());
    g_string_append(signature_string, method);
    g_string_append(signature_string, path);
    g_string_append_printf(signature_string, "%ld", ts);

    unsigned int hash_len = 128;
    char hash_out[128] = {0};
    // HMAC_CTX hmac;
    // HMAC_CTX_init(&hmac);
    // HMAC_Init_ex(&hmac, secret, (int) strlen(secret), EVP_sha256(), NULL);
    // HMAC_Update(&hmac, (const guchar *) signature_string->str, signature_string->len);
    HMAC_CTX *hmac;

#if OPENSSL_VERSION_NUMBER < 0x10100000L
	HMAC_CTX hmac_l;
	HMAC_CTX_init(&hmac_l);
	hmac = &hmac_l;
#else
	hmac = HMAC_CTX_new();
#endif
	HMAC_Init_ex(hmac, secret, (int) strlen(secret), EVP_sha256(), nullptr);
	HMAC_Update(hmac, (const guchar *) signature_string->str, signature_string->len);


    // HMAC_Final(&hmac, (unsigned char *) hash_out, &hash_len);
    // HMAC_cleanup(&hmac);
    HMAC_Final(hmac, (unsigned char *) hash_out, &hash_len);
	// HMAC_CTX_cleanup(hmac);
#if OPENSSL_VERSION_NUMBER < 0x10100000L
	HMAC_CTX_cleanup(hmac);
#else
	HMAC_CTX_free(hmac);
#endif

// #if OPENSSL_VERSION_NUMBER >= 0x10100000L
// 	HMAC_CTX_free(hmac);
// #endif
    gchar *signature = digest_to_string((guchar *) hash_out, 32);
    g_string_free(signature_string, TRUE);

    char APP[32];
    char KEY[128];
    char SIGNATURE[128];
    char TIMESTAMP[32];
    snprintf(APP, 32, "X-API-APP: %s", DeviceSerialNumber.c_str());
    snprintf(KEY, 128, "X-API-KEY: %s", key);
    snprintf(SIGNATURE, 128, "X-API-SIGNATURE: %s", signature);
    snprintf(TIMESTAMP, 32, "X-API-TIMESTAMP: %ld", ts);

    request.AppendHeader(APP);
    request.AppendHeader(KEY);
    request.AppendHeader(SIGNATURE);
    request.AppendHeader(TIMESTAMP);

    g_free(signature);

    return 0;
}

#define RETURN_NEGATIVE_IF_FAIL(request) {if (request.JSONResult.HasParseError()) { return -1; }}
#define JSON(request) request.JSONResult

WSAPI::WSAPI()
{
    ilock = PTHREAD_MUTEX_INITIALIZER;
    https = new HttpClient;
    renew_key();
}

WSAPI::~WSAPI()
{
    delete https;
}

int WSAPI::Register(DeviceBase &i)
{
    Document Json;
    i.ToJson(Json);

    string url = APIBase;
    url += "/api/v1/devices/register";

    HttpRequest request(NULL, NULL, false);
    send_request(request, &Json, url.c_str(), "POST");
    request.ToJson();
    RETURN_NEGATIVE_IF_FAIL(request);

    const Value& response = JSON(request);
    Value::ConstMemberIterator itr = response.FindMember("data");
    if (itr == response.MemberEnd()) {
        return -1;
    } else {
        const Value& data = response["data"];

        Value::ConstMemberIterator itr_data = data.FindMember("api-key");
        if ((itr_data == data.MemberEnd()) || (!itr_data->value.IsString())) {
            return -1;
        }
        const char *key = itr_data->value.GetString();

        itr_data = data.FindMember("api-secret");
        if ((itr_data == data.MemberEnd()) || (!itr_data->value.IsString())) {
            return -1;
        }
        const char *secret = itr_data->value.GetString();

        save_key_pair(key, secret);
        renew_key();
    }

    return request.ResponseCode;
}

int WSAPI::SendData(Document *d)
{
    string url = APIBase;    
    string path = "/api/v1/devices/";
    path += DeviceSerialNumber;
    path += "/dataset";
    url += path;

    HttpRequest request(NULL, NULL, false);
    signature(key.c_str(), secret.c_str(), "PUT", path.c_str(), request);
    send_request(request, d, url.c_str(), "PUT");
    request.ToJson();
    RETURN_NEGATIVE_IF_FAIL(request);

    return request.ResponseCode;
}

int WSAPI::SendStatus(DeviceStatus &i)
{
    Document Json;
    i.ToJson(Json);

    string url = APIBase;
    string path = "/api/v1/devices/";
    path += DeviceSerialNumber;
    path += "/status";
    url += path;

    HttpRequest request(NULL, NULL, false);
    signature(key.c_str(), secret.c_str(), "PUT", path.c_str(), request);
    send_request(request, &Json, url.c_str(), "PUT");
    request.ToJson();
    RETURN_NEGATIVE_IF_FAIL(request);

    return request.ResponseCode;
}

int WSAPI::SendPing(DevicePing &i)
{
    Document Json;
    i.ToJson(Json);

    string url = APIBase;
    url += "/api/v1/devices/";
    url += DeviceSerialNumber;
    url += "/heartbeat";

    HttpRequest request(NULL, NULL, false);
    send_request(request, &Json, url.c_str(), "PUT");
    request.ToJson();
    RETURN_NEGATIVE_IF_FAIL(request);

    return request.ResponseCode;
}

int WSAPI::SendCmdFeedback(DeviceCmdFeedback &i)
{
    Document Json;
    i.ToJson(Json);

    string url = APIBase;
    string path = "/api/v1/device/cmdLog/result";
    url += path;

    HttpRequest request(NULL, NULL, false);
    signature(key.c_str(), secret.c_str(), "POST", path.c_str(), request);
    send_request(request, &Json, url.c_str(), "POST");
    request.ToJson();
    RETURN_NEGATIVE_IF_FAIL(request);

    return request.ResponseCode;
}

int WSAPI::RequestCert()
{

}

void WSAPI::renew_key()
{
    key = get_key();
    secret = get_secret();
}

int WSAPI::send_request(HttpRequest &request, Document *Json, const char *api, const char *method)
{
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    Json->Accept(writer);
    const char *output = buffer.GetString();

    request.RequestUrl = g_string_new(api);
    request.RequestData = g_string_new(output);
    request.AppendHeader("Content-Type: application/json");
    lock();
    https->REQUEST(&request, method);
    unlock();

    return 0;
}
