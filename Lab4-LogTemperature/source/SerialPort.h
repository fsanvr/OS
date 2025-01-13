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


/**
 * @struct SerialPort
 * @brief Структура для управления последовательным портом.
 */
typedef struct {
#ifdef _WIN32
    HANDLE handle;
#else
    int fd;
#endif
    char portName[20];
} SerialPort;


/**
 * @brief Открывает и настраивает последовательный порт.
 *
 * @param portName Имя порта (например, "COM7" или "/dev/ttyS0").
 * @param baudRate Скорость передачи данных (9600, 19200 и т. д.).
 * @return Указатель на структуру SerialPort или NULL в случае ошибки.
 */
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

/**
 * @brief Закрывает последовательный порт.
 *
 * @param serial Указатель на структуру SerialPort.
 */
void SerialClose(SerialPort *serial) {
    if (!serial) return;

#ifdef _WIN32
    CloseHandle(serial->handle);
#else
    close(serial->fd);
#endif

    free(serial);
}

/**
 * @brief Записывает данные в последовательный порт.
 *
 * @param serial Указатель на структуру SerialPort.
 * @param data Данные для записи.
 * @param length Длина данных.
 * @return Количество записанных байтов или -1 в случае ошибки.
 */
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

/**
 * @brief Читает данные из последовательного порта.
 *
 * @param serial Указатель на структуру SerialPort.
 * @param buffer Буфер для хранения прочитанных данных.
 * @param bufferSize Размер буфера.
 * @return Количество прочитанных байтов или -1 в случае ошибки.
 */
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

#endif // SERIAL_H