#ifndef HTTP_H
#define HTTP_H

#include <pthread.h>

#include <curl/curl.h>
#include <glib-2.0/glib.h>
#include <rapidjson/document.h>

using namespace std;
using namespace rapidjson;

class HttpRequest {
public:
    struct curl_slist *Headers;
    GString *RequestUrl;
    GString *RequestData;
    bool EnableCompress;

public:
    long ResponseCode;
    GString *ResponseBuf;
    Document JSONResult;

private:
    CURLcode status;

public:
    HttpRequest(GString *url, GString *data, bool use_gzip):
        Headers(NULL),
        RequestUrl(url),
        RequestData(data),
        EnableCompress(use_gzip),
        status(CURL_LAST)
    {
        ResponseBuf = g_string_sized_new(131072);
    }

    ~HttpRequest()
    {
        g_string_free(RequestUrl, TRUE);
        if (RequestData) g_string_free(RequestData, TRUE);
        g_string_free(ResponseBuf, TRUE);
        curl_slist_free_all(Headers);
    }

    void AppendHeader(const char *header) { if (header) Headers = curl_slist_append(Headers, header); }

    ParseErrorCode ToJsonInsitu() { JSONResult.ParseInsitu<kParseStopWhenDoneFlag>(ResponseBuf->str); return JSONResult.GetParseError(); }
    ParseErrorCode ToJsonInsituAsString() { JSONResult.Parse<kParseStopWhenDoneFlag|kParseNumbersAsStringsFlag>(ResponseBuf->str); return JSONResult.GetParseError(); }
    ParseErrorCode ToJson() { JSONResult.Parse<kParseStopWhenDoneFlag>(ResponseBuf->str, ResponseBuf->len); return JSONResult.GetParseError(); }

    CURLcode GetStatus() const { return status; }
    void SetStatus(CURLcode code) { status = code; }
};

typedef struct {
    pthread_mutex_t connect;
    pthread_mutex_t ssl_session;
    pthread_mutex_t dns_cache;
    pthread_mutex_t cookie;
} HttpSharedLockSet;

class HttpClient {
private:
    pthread_mutex_t internal_lock;
    HttpSharedLockSet shared_locks;
    CURLSH *shared_obj;

public:
    HttpClient(void);
    ~HttpClient(void);

public:
    CURLcode REQUEST(HttpRequest *request, const char *method);

private:
    void lock_client(void) { pthread_mutex_lock(&internal_lock); }
    void unlock_client(void) { pthread_mutex_unlock(&internal_lock); }
};

#endif // HTTP_H
