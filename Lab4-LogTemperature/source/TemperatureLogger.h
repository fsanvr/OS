#ifndef TEMPERATURE_LOGGER_H
#define TEMPERATURE_LOGGER_H

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


/**
 * @struct TemperatureLogger
 * @brief Структура для логирования данных о температуре с последовательного порта.
 */
typedef struct {
    SerialPort *serialPort;
    const char *portName;
    int baudRate;
    char logFilePath[256];
    char hourlyLogFilePath[256];
    char dailyLogFilePath[256];
    time_t endTime;
} TemperatureLogger;


/**
 * @brief Функция для записи строки в лог-файл.
 *
 * @param filePath Путь к файлу.
 * @param data Строка для записи.
 */
void WriteToFile(const char *filePath, const char *data) {
    FILE *file = fopen(filePath, "a+");
    if (file) {
        fprintf(file, "%s\n", data);
        fclose(file);
    } else {
        perror("Ошибка: файл не открыт.\n");
    }
}

/**
 * @brief Удаляет старые записи из лог-файла, оставляя только записи за последние N часов.
 *
 * @param filePath Путь к файлу.
 * @param maxHours Максимальное количество часов хранения.
 */
void CleanupLogFile(const char *filePath, int maxHours) {
    FILE *file = fopen(filePath, "r");
    if (!file) return;

    char lines[1024][256];
    int count = 0;
    time_t now = time(NULL);
    struct tm entryTime;

    while (fgets(lines[count], sizeof(lines[count]), file)) {
        sscanf(lines[count], "%4d-%2d-%2d %2d:", &entryTime.tm_year, &entryTime.tm_mon, &entryTime.tm_mday, &entryTime.tm_hour);
        entryTime.tm_year -= 1900;
        entryTime.tm_mon -= 1;
        entryTime.tm_min = entryTime.tm_sec = 0;

        if (difftime(now, mktime(&entryTime)) <= maxHours * 3600) {
            count++;
        }
    }
    fclose(file);

    file = fopen(filePath, "w");
    if (file) {
        for (int i = 0; i < count; i++) {
            fprintf(file, "%s", lines[i]);
        }
        fclose(file);
    }
}

/**
 * @brief Инициализирует логгер температуры.
 *
 * @param portName Имя последовательного порта (например, "COM8" или "/dev/ttyUSB0").
 * @param baudRate Скорость передачи данных.
 * @param logDir Папка для хранения лог-файлов.
 * @return Указатель на структуру TemperatureLogger или NULL в случае ошибки.
 */
TemperatureLogger* TemperatureLoggerInit(
    const char *portName, int baudRate, const char * logFilePath, const char * hourlyLogFilePath, const char * dailyLogFilePath,
    time_t endTime
    ) {
    TemperatureLogger *logger = (TemperatureLogger *)malloc(sizeof(TemperatureLogger));
    if (!logger) return NULL;

    logger->portName = portName;
    logger->baudRate = baudRate;
    strncpy(logger->logFilePath, logFilePath, sizeof(logger->logFilePath) - 1);
    strncpy(logger->hourlyLogFilePath, hourlyLogFilePath, sizeof(logger->hourlyLogFilePath) - 1);
    strncpy(logger->dailyLogFilePath, dailyLogFilePath, sizeof(logger->dailyLogFilePath) - 1);
    logger->logFilePath[sizeof(logger->logFilePath) - 1] = '\0';
    logger->hourlyLogFilePath[sizeof(logger->hourlyLogFilePath) - 1] = '\0';
    logger->dailyLogFilePath[sizeof(logger->dailyLogFilePath) - 1] = '\0';
    logger->endTime = endTime;

    FILE *log = fopen(logger->logFilePath, "w");
    FILE *hourlyLog = fopen(logger->hourlyLogFilePath, "w");
    FILE *dailyLog = fopen(logger->dailyLogFilePath, "w");

    if (!log || !hourlyLog || !dailyLog) {
      perror("Ошибка: файлы логов не созданы.\n");
    }

    logger->serialPort = SerialOpen(portName, baudRate);
    if (!logger->serialPort) {
        free(logger);
        return NULL;
    }

    return logger;
}

/**
 * @brief Завершает работу логгера температуры и освобождает ресурсы.
 *
 * @param logger Указатель на TemperatureLogger.
 */
void TemperatureLoggerClose(TemperatureLogger *logger) {
    if (logger) {
        SerialClose(logger->serialPort);
        free(logger);
    }
}

/**
 * @brief Записывает измеренную температуру в лог-файл.
 */
static void LogTemperature(TemperatureLogger *logger, int temperature) {
    time_t now = time(NULL);
    struct tm *tmNow = localtime(&now);

    char logEntry[64];
    snprintf(logEntry, sizeof(logEntry), "%4d-%02d-%02d %02d:%02d:%02d %d",
             tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday,
             tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, temperature);
    WriteToFile(logger->logFilePath, logEntry);
}

/**
 * @brief Обновляет среднюю температуру за час.
 */
static void UpdateHourlyAverage(TemperatureLogger *logger, int *hourlySum, int *hourlyCount, time_t *lastHour) {
    if (*hourlyCount > 0) {
        time_t now = time(NULL);
        struct tm *tmNow = localtime(&now);

        char hourlyEntry[64];
        snprintf(hourlyEntry, sizeof(hourlyEntry), "%4d-%02d-%02d %02d:00 %d",
                 tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday,
                 tmNow->tm_hour, *hourlySum / *hourlyCount);
        WriteToFile(logger->hourlyLogFilePath, hourlyEntry);
    }
    
    *hourlySum = 0;
    *hourlyCount = 0;
    *lastHour = time(NULL);
}

/**
 * @brief Обновляет среднюю температуру за день.
 */
static void UpdateDailyAverage(TemperatureLogger *logger, int *dailySum, int *dailyCount, time_t *lastDay) {
    if (*dailyCount > 0) {
        time_t now = time(NULL);
        struct tm *tmNow = localtime(&now);

        char dailyEntry[64];
        snprintf(dailyEntry, sizeof(dailyEntry), "%4d-%02d-%02d %d",
                 tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday,
                 *dailySum / *dailyCount);
        WriteToFile(logger->dailyLogFilePath, dailyEntry);
    }
    
    *dailySum = 0;
    *dailyCount = 0;
    *lastDay = time(NULL);
}

/**
 * @brief Обрабатывает данные о температуре.
 */
static void ProcessTemperatureData(TemperatureLogger *logger, int temperature, int *hourlySum, int *hourlyCount, 
                                   int *dailySum, int *dailyCount, time_t *lastHour, time_t *lastDay) {
    time_t now = time(NULL);
    
    *hourlySum += temperature;
    (*hourlyCount)++;
    *dailySum += temperature;
    (*dailyCount)++;

    LogTemperature(logger, temperature);

    if (difftime(now, *lastHour) >= 3600) {
        UpdateHourlyAverage(logger, hourlySum, hourlyCount, lastHour);
    }

    if (difftime(now, *lastDay) >= 86400) {
        UpdateDailyAverage(logger, dailySum, dailyCount, lastDay);
    }
}

/**
 * @brief Запускает процесс логирования температуры.
 *
 * @param logger Указатель на TemperatureLogger.
 */
void TemperatureLoggerRun(TemperatureLogger *logger) {
    time_t lastHour = time(NULL);
    time_t lastDay = time(NULL);
    int hourlySum = 0, hourlyCount = 0;
    int dailySum = 0, dailyCount = 0;
    
    while (time(NULL) < logger->endTime) {
      char buffer[32];
      if (SerialRead(logger->serialPort, buffer, sizeof(buffer) - 1)) {
          int temperature = atoi(buffer);
          ProcessTemperatureData(logger, temperature, &hourlySum, &hourlyCount, &dailySum, &dailyCount, &lastHour, &lastDay);
      }
      SleepMs(1000);
    }
}

#endif // TEMPERATURE_LOGGER_H