#ifndef PROFILES_H
#define PROFILES_H

#include <string>

using namespace std;

string get_ethaddr();

string get_device_sn();

string get_systime();

string get_key();

string get_secret();

void save_key_pair(const char *key, const char *secret);

#endif // PROFILES_H
