#include "SerialPort.h"

SerialPort* SerialOpen(const char *portName, int baudRate) {
    SerialPort *serial = (SerialPort*)malloc(sizeof(SerialPort));
    if (!serial) return NULL;

    strncpy(serial->portName, portName, sizeof(serial->portName) - 1);
    serial->portName[sizeof(serial->portName) - 1] = '\0';

#ifdef _WIN32
    serial->handle = CreateFile(portName, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);
    if (serial->handle == INVALID_HANDLE_VALUE) {
        free(serial);
        return NULL;
    }

    DCB dcbSerialParams = {0};
    if (!GetCommState(serial->handle, &dcbSerialParams)) {
        CloseHandle(serial->handle);
        free(serial);
        return NULL;
    }

    dcbSerialParams.BaudRate = baudRate;
    dcbSerialParams.ByteSize = 8;
    dcbSerialParams.StopBits = ONESTOPBIT;
    dcbSerialParams.Parity = NOPARITY;

    if (!SetCommState(serial->handle, &dcbSerialParams)) {
        CloseHandle(serial->handle);
        free(serial);
        return NULL;
    }

    COMMTIMEOUTS timeouts = {0};
    timeouts.ReadIntervalTimeout = 50;
    timeouts.ReadTotalTimeoutConstant = 1000;
    timeouts.ReadTotalTimeoutMultiplier = 10;
    timeouts.WriteTotalTimeoutConstant = 1000;
    timeouts.WriteTotalTimeoutMultiplier = 10;

    if (!SetCommTimeouts(serial->handle, &timeouts)) {
        CloseHandle(serial->handle);
        free(serial);
        return NULL;
    }

#else
    serial->fd = open(portName, O_RDWR | O_NOCTTY | O_NDELAY);
    if (serial->fd == -1) {
        free(serial);
        return NULL;
    }

    struct termios options;
    tcgetattr(serial->fd, &options);

    cfsetispeed(&options, baudRate);
    cfsetospeed(&options, baudRate);

    options.c_cflag = CS8 | CLOCAL | CREAD;
    options.c_iflag = IGNPAR;
    options.c_oflag = 0;
    options.c_lflag = 0;

    tcflush(serial->fd, TCIFLUSH);
    tcsetattr(serial->fd, TCSANOW, &options);
#endif

    return serial;
}

void SerialClose(SerialPort *serial) {
    if (!serial) return;

#ifdef _WIN32
    CloseHandle(serial->handle);
#else
    close(serial->fd);
#endif

    free(serial);
}

int SerialWrite(SerialPort *serial, const char *data, size_t length) {
    if (!serial) return -1;

#ifdef _WIN32
    DWORD bytesWritten;
    if (!WriteFile(serial->handle, data, length, &bytesWritten, NULL)) {
        return -1;
    }
    return (int)bytesWritten;
#else
    int bytesWritten = write(serial->fd, data, length);
    return (bytesWritten < 0) ? -1 : bytesWritten;
#endif
}

int SerialRead(SerialPort *serial, char *buffer, size_t bufferSize) {
    if (!serial) return -1;

#ifdef _WIN32
    DWORD bytesRead;
    if (ReadFile(serial->handle, buffer, bufferSize - 1, &bytesRead, NULL) && bytesRead > 0) {
        buffer[bytesRead] = '\0';
        return (int)bytesRead;
    }
    return -1;
#else
    int bytesRead = read(serial->fd, buffer, bufferSize - 1);
    if (bytesRead > 0) {
        buffer[bytesRead] = '\0';
    }
    return (bytesRead < 0) ? -1 : bytesRead;
#endif
}