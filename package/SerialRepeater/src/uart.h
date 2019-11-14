#ifndef UART_H
#define UART_H

int serial_open(const char *dev, int baud);

int serial_open_even(const char *dev, int baud);

int serial_close(int fd);

int serial_send(int fd, const char *msg, int len);

#endif // UART_H
