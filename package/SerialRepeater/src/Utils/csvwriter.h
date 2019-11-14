#ifndef CSVWRITER_H
#define CSVWRITER_H

#include <string>
#include <vector>
#include <queue>
#include <pthread.h>

#include <evloop.h>

using namespace std;

typedef vector<string> csvRowData;

class CSVLogger {
public:
    static string fromInt32     (int n);
    static string fromInt64     (long long n);
    static string fromLInt      (long n);
    static string fromUInt32    (unsigned int n);
    static string fromUInt64    (unsigned long long n);
    static string fromULInt     (unsigned long n);
    static string fromFloat     (float n);
    static string fromDouble    (double n);
    static string fromBool      (bool v);

private:
    queue<string>       contents;
    string              filename;
    EventLoop           *evloop;
    csvRowData          header;
    pthread_t           tid;

public:
    CSVLogger(const char *file);
    CSVLogger(const char *file, csvRowData &header);
    ~CSVLogger();

    void Insert(csvRowData &data);
    void Flush();
    void Start();
    void Stop();
    void Dispatch();
};

#endif // CSVWRITER_H
