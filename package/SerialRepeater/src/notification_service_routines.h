#ifndef NOTIFICATION_SERVICE_ROUTINES_H
#define NOTIFICATION_SERVICE_ROUTINES_H

void terminalcrtl_nsr(const char *payload);

void syscrtl_nsr(const char *payload);

void multicast_nsr(const char *payload);

void modbus_over_http_nsr(const char *payload);

#endif // NOTIFICATION_SERVICE_ROUTINES_H
