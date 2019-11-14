#include "http.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <curl/curl.h>

#include <common_defs.h>
#include <delay.hpp>

static size_t received_buffer_writer(void *contents, size_t size, size_t nmemb, void *buf)
{
    if (!contents || !buf) return 0;
    size_t total = size * nmemb;
    GString *recv_buf = (GString*) buf;
    g_string_append_len(recv_buf, (const gchar*) contents, total);
    return total;
}

static void lock_shared(CURL *handle, curl_lock_data data, curl_lock_access access, void *userptr)
{
    (void)access;
    (void)handle;

    HttpSharedLockSet *locks = (HttpSharedLockSet *) userptr;

    switch (data) {
    case CURL_LOCK_DATA_CONNECT:
        pthread_mutex_lock(&locks->connect);
        break;
    case CURL_LOCK_DATA_DNS:
        pthread_mutex_unlock(&locks->dns_cache);
        break;
    case CURL_LOCK_DATA_SSL_SESSION:
        pthread_mutex_unlock(&locks->ssl_session);
        break;
    default:
        break;
    }
}

static void unlock_shared(CURL *handle, curl_lock_data data, void *userptr)
{
    (void)handle;

    HttpSharedLockSet *locks = (HttpSharedLockSet *) userptr;

    switch (data) {
    case CURL_LOCK_DATA_CONNECT:
        pthread_mutex_unlock(&locks->connect);
        break;
    case CURL_LOCK_DATA_DNS:
        pthread_mutex_unlock(&locks->dns_cache);
        break;
    case CURL_LOCK_DATA_SSL_SESSION:
        pthread_mutex_unlock(&locks->ssl_session);
        break;
    default:
        break;
    }
}

HttpClient::HttpClient(void)
{
    curl_global_init(CURL_GLOBAL_ALL);

    internal_lock = PTHREAD_MUTEX_INITIALIZER;
    shared_locks.connect = PTHREAD_MUTEX_INITIALIZER;
    shared_locks.ssl_session = PTHREAD_MUTEX_INITIALIZER;
    shared_locks.dns_cache = PTHREAD_MUTEX_INITIALIZER;
    shared_locks.cookie = PTHREAD_MUTEX_INITIALIZER;

    shared_obj = curl_share_init();
    curl_share_setopt(shared_obj, CURLSHOPT_LOCKFUNC, lock_shared);
    curl_share_setopt(shared_obj, CURLSHOPT_UNLOCKFUNC, unlock_shared);
    curl_share_setopt(shared_obj, CURLSHOPT_USERDATA, &shared_locks);
    curl_share_setopt(shared_obj, CURLSHOPT_SHARE, CURL_LOCK_DATA_CONNECT);
    curl_share_setopt(shared_obj, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    curl_share_setopt(shared_obj, CURLSHOPT_SHARE, CURL_LOCK_DATA_SSL_SESSION);
}

HttpClient::~HttpClient(void)
{
    curl_share_cleanup(shared_obj);
    curl_global_cleanup();
}

CURLcode HttpClient::REQUEST(HttpRequest *request, const char *method)
{
    CURLcode ret = CURL_LAST;
    CURL *curl = curl_easy_init();
    if (!curl) { return ret; }

#ifdef HTTP_DEBUG
    if (debug_mode) fprintf(stderr, "[URL]:\n%s\n", request->RequestUrl->str);
#endif

    curl_easy_setopt(curl, CURLOPT_SHARE, shared_obj);
    curl_easy_setopt(curl, CURLOPT_URL, request->RequestUrl->str);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, received_buffer_writer);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *) request->ResponseBuf);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
    curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);
    curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);

    if (request->Headers) curl_easy_setopt(curl, CURLOPT_HTTPHEADER, request->Headers);

    if (request->EnableCompress) curl_easy_setopt(curl, CURLOPT_ACCEPT_ENCODING, "");

    if (!strcmp(method, "POST")) {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
    }

    if (!strcmp(method, "PUT")) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
    }

    if (!strcmp(method, "DELETE")) {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }

    if (request->RequestData) {
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request->RequestData->str);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, (long) request->RequestData->len);
#ifdef HTTP_DEBUG
        if (debug_mode) fprintf(stderr, "[Content]:\n%s\n", request->RequestData->str);
#endif
    }

    ret = curl_easy_perform(curl);

    request->SetStatus(ret);
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &request->ResponseCode);
#ifdef HTTP_DEBUG
    if (debug_mode) fprintf(stderr, "[Response %d]:\n%s\n", request->ResponseCode, request->ResponseBuf->str);
#endif
    curl_easy_cleanup(curl);

    return ret;
}
