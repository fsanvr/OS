#include "TemperatureDeviceSimulator.h"

double generateRandomTemperature(double minTemperature, double maxTemperature) {
    double range = (maxTemperature - minTemperature); 
    double div = RAND_MAX / range;
    return minTemperature + (rand() / div);
}

TemperatureDeviceSimulator* TemperatureDeviceSimulatorInit(
    const char *portName, double baudRate, double minTemperature, int maxTemperature, int intervalMs
    ) {
    TemperatureDeviceSimulator *temperatureDeviceSimulator = (TemperatureDeviceSimulator *)malloc(sizeof(TemperatureDeviceSimulator));
    if (!temperatureDeviceSimulator) {
        return NULL;
    }
    
    temperatureDeviceSimulator->portName = portName;
    temperatureDeviceSimulator->baudRate = baudRate;
    temperatureDeviceSimulator->minTemperature = minTemperature;
    temperatureDeviceSimulator->maxTemperature = maxTemperature;
    temperatureDeviceSimulator->intervalMs = intervalMs;

    temperatureDeviceSimulator->serialPort = SerialOpen(portName, baudRate);
    if (!temperatureDeviceSimulator->serialPort) {
        free(temperatureDeviceSimulator);
        return NULL;
    }

    srand(time(NULL));

    return temperatureDeviceSimulator;
}

void TemperatureDeviceSimulatorClose(TemperatureDeviceSimulator *temperatureDeviceSimulator) {
    if (temperatureDeviceSimulator) {
        if (temperatureDeviceSimulator->serialPort) {
            SerialClose(temperatureDeviceSimulator->serialPort);
        }
        free(temperatureDeviceSimulator);
    }
}

void TemperatureDeviceSimulatorRun(TemperatureDeviceSimulator *temperatureDeviceSimulator) {
    if (!temperatureDeviceSimulator || !temperatureDeviceSimulator->serialPort) {
        return;
    }

    char temperatureBuffer[TEMPERATURE_BUFFER_SIZE];
    
    while (1) {
        double temperature = generateRandomTemperature(temperatureDeviceSimulator->minTemperature, temperatureDeviceSimulator->maxTemperature);

        snprintf(temperatureBuffer, TEMPERATURE_BUFFER_SIZE, "%f\n", temperature);

        int bytesWritten = SerialWrite(temperatureDeviceSimulator->serialPort, temperatureBuffer, strlen(temperatureBuffer));
        if (bytesWritten < 0) {
            printf("Ошибка записи на порт %s\n", temperatureDeviceSimulator->portName);
            break;
        }

        printf("Отправлено в порт %s: %s", temperatureDeviceSimulator->portName, temperatureBuffer);

        SleepMs(temperatureDeviceSimulator->intervalMs);
    }
}