#ifndef TEMPERATURE_DEVICE_SIMULATOR_H
#define TEMPERATURE_DEVICE_SIMULATOR_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "SerialPort.h"

#ifdef _WIN32
    #include <windows.h>
    #define SleepMs(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define SleepMs(ms) usleep((ms) * 1000)
#endif

#define TEMPERATURE_BUFFER_SIZE 32

typedef struct {
    SerialPort *serialPort;
    const char *portName;
    int baudRate;
    double minTemperature;
    double maxTemperature;
    int intervalMs;
} TemperatureDeviceSimulator;

double generateRandomTemperature(double minTemperature, double maxTemperature);

TemperatureDeviceSimulator* TemperatureDeviceSimulatorInit(
    const char *portName,
    double baudRate,
    double minTemperature,
    int maxTemperature,
    int intervalMs
    );

void TemperatureDeviceSimulatorClose(TemperatureDeviceSimulator *temperatureDeviceSimulator);

void TemperatureDeviceSimulatorRun(TemperatureDeviceSimulator *temperatureDeviceSimulator);

#endif // TEMPERATURE_DEVICE_SIMULATOR_H