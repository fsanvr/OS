#ifndef SERIAL_H
#define SERIAL_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <termios.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <errno.h>
#endif

typedef struct {
#ifdef _WIN32
    HANDLE handle;
#else
    int fd;
#endif
    char portName[20];
} SerialPort;

SerialPort* SerialOpen(const char *portName, int baudRate);

void SerialClose(SerialPort *serial);

int SerialWrite(SerialPort *serial, const char *data, size_t length);

int SerialRead(SerialPort *serial, char *buffer, size_t bufferSize);

#endif // SERIAL_H
