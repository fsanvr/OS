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


/**
 * @struct TemperatureDeviceSimulator
 * @brief Структура для эмуляции устройства, передающего данные о температуре по последовательному порту.
 */
typedef struct {
    SerialPort *serialPort;
    const char *portName;
    int baudRate;
    int minTemperature;
    int maxTemperature;
    int intervalMs;
    time_t endTime;
} TemperatureDeviceSimulator;

/**
 * @brief Генерирует случайное значение температуры в указанном диапазоне.
 *
 * @param minTemperature Минимальное значение температуры.
 * @param maxTemperature Максимальное значение температуры.
 *
 * @return Случайное значение температуры в пределах от minTemperature до maxTemperature.
 */
static int generateRandomTemperature(int minTemperature, int maxTemperature) {
    return rand() % (maxTemperature - minTemperature + 1) + minTemperature;
}

/**
 * @brief Инициализирует симулятор устройства температуры и открывает последовательный порт.
 *
 * @param portName Имя последовательного порта.
 * @param baudRate Скорость передачи данных.
 * @param minTemperature Минимальная температура для генерации.
 * @param maxTemperature Максимальная температура для генерации.
 * @param intervalMs Интервал записи данных в миллисекундах.
 * @param endTime Время окончания работы симулятора.
 *
 * @return Указатель на структуру TemperatureDeviceSimulator, если инициализация успешна.
 *         В случае ошибки возвращает NULL.
 */
TemperatureDeviceSimulator *TemperatureDeviceSimulatorInit(
    const char *portName, int baudRate, int minTemperature, int maxTemperature, int intervalMs, time_t endTime
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
    temperatureDeviceSimulator->endTime = endTime;

    temperatureDeviceSimulator->serialPort = SerialOpen(portName, baudRate);
    if (!temperatureDeviceSimulator->serialPort) {
        free(temperatureDeviceSimulator);
        return NULL;
    }

    srand(time(NULL));

    return temperatureDeviceSimulator;
}

/**
 * @brief Закрывает симулятор устройства температуры, освобождая все ресурсы.
 * 
 * После вызова этой функции последовательный порт будет закрыт, а все связанные ресурсы - освобождены.
 *
 * @param temperatureDeviceSimulator Указатель на структуру TemperatureDeviceSimulator.
 */
void TemperatureDeviceSimulatorClose(TemperatureDeviceSimulator *temperatureDeviceSimulator) {
    if (temperatureDeviceSimulator) {
        if (temperatureDeviceSimulator->serialPort) {
            SerialClose(temperatureDeviceSimulator->serialPort);
        }
        free(temperatureDeviceSimulator);
    }
}

/**
 * @brief Запускает симуляцию устройства температуры.
 *
 * @param temperatureDeviceSimulator Указатель на структуру TemperatureDeviceSimulator.
 */
void TemperatureDeviceSimulatorRun(TemperatureDeviceSimulator *temperatureDeviceSimulator) {
    if (!temperatureDeviceSimulator || !temperatureDeviceSimulator->serialPort) {
        return;
    }

    char temperatureBuffer[TEMPERATURE_BUFFER_SIZE];
    
    while (time(NULL) < temperatureDeviceSimulator->endTime) {
        int temperature = generateRandomTemperature(temperatureDeviceSimulator->minTemperature, temperatureDeviceSimulator->maxTemperature);

        snprintf(temperatureBuffer, TEMPERATURE_BUFFER_SIZE, "Temperature: %d\n", temperature);

        int bytesWritten = SerialWrite(temperatureDeviceSimulator->serialPort, temperatureBuffer, strlen(temperatureBuffer));
        if (bytesWritten < 0) {
            printf("Ошибка записи на порт %s\n", temperatureDeviceSimulator->portName);
            break;
        }

        printf("Отправлено в порт %s: %s", temperatureDeviceSimulator->portName, temperatureBuffer);

        SleepMs(temperatureDeviceSimulator->intervalMs);
    }
}

#endif // TEMPERATURE_DEVICE_SIMULATOR_