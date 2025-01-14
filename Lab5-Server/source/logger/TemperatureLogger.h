#ifndef TEMPERATURE_LOGGER_H
#define TEMPERATURE_LOGGER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "SerialPort.h"
#include "../database/Database.h"

#ifdef _WIN32
    #include <windows.h>
    #define SleepMs(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SleepMs(ms) usleep((ms) * 1000)
#endif

typedef struct {
    SerialPort *serialPort;
    const char *portName;
    int baudRate;
    char logFilePath[256];
    char hourlyLogFilePath[256];
    char dailyLogFilePath[256];
} TemperatureLogger;

TemperatureLogger* TemperatureLoggerInit(
    const char *portName,
    int baudRate,
    const char *logFilePath,
    const char *hourlyLogFilePath,
    const char *dailyLogFilePath
    );

void TemperatureLoggerClose(TemperatureLogger *logger);

void TemperatureLoggerRun(TemperatureLogger *logger);

#endif // TEMPERATURE_LOGGER_H