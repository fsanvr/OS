#include "TemperatureDeviceSimulator.h"

double generateRandomTemperature(double minTemperature, double maxTemperature) {
    double range = (maxTemperature - minTemperature); 
    double div = RAND_MAX / range;
    return minTemperature + (rand() / div);
}

double smoothedTemperature(double previousTemperature, double minTemperature, double maxTemperature, double alpha) {
    double newTemperature = generateRandomTemperature(minTemperature, maxTemperature);
    return previousTemperature * (1 - alpha) + newTemperature * alpha;
}

TemperatureDeviceSimulator* TemperatureDeviceSimulatorInit(
    const char *portName, double baudRate, double minTemperature, double maxTemperature, double alpha, int intervalMs
    ) {
    TemperatureDeviceSimulator *temperatureDeviceSimulator = (TemperatureDeviceSimulator *)malloc(sizeof(TemperatureDeviceSimulator));
    if (!temperatureDeviceSimulator) {
        return NULL;
    }
    
    temperatureDeviceSimulator->portName = portName;
    temperatureDeviceSimulator->baudRate = baudRate;
    temperatureDeviceSimulator->minTemperature = minTemperature;
    temperatureDeviceSimulator->maxTemperature = maxTemperature;
    temperatureDeviceSimulator->alpha = alpha;
    temperatureDeviceSimulator->intervalMs = intervalMs;
    temperatureDeviceSimulator->previousTemperature = 0.0;

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
        double temperature = smoothedTemperature(
            temperatureDeviceSimulator->previousTemperature,
            temperatureDeviceSimulator->minTemperature,
            temperatureDeviceSimulator->maxTemperature,
            temperatureDeviceSimulator->alpha
            );

        snprintf(temperatureBuffer, TEMPERATURE_BUFFER_SIZE, "%f\n", temperature);

        int bytesWritten = SerialWrite(temperatureDeviceSimulator->serialPort, temperatureBuffer, strlen(temperatureBuffer));
        if (bytesWritten < 0) {
            printf("Ошибка записи на порт %s\n", temperatureDeviceSimulator->portName);
            break;
        }

        // printf("Отправлено в порт %s: %s", temperatureDeviceSimulator->portName, temperatureBuffer);

        SleepMs(temperatureDeviceSimulator->intervalMs);
    }
}