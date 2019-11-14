#ifndef SYSCONF_H
#define SYSCONF_H

#include <string>

using namespace std;

string sysconf_get(const char *key);

int sysconf_set(const char *key, const char *value);

int sysconf_del(const char *key);

#endif // SYSCONF_H
