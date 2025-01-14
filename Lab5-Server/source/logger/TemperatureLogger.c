#include "TemperatureLogger.h"

void WriteToFile(const char *filePath, const char *data) {
    FILE *file = fopen(filePath, "a+");
    if (file) {
        fprintf(file, "%s\n", data);
        fclose(file);
    } else {
        perror("Ошибка: файл не открыт.\n");
    }
}

void WriteToDatabase(double temperature) {
    bool result = database_insert_temperature(temperature);

    if (!result) {
        perror("Ошибка записи температуры в базу данных.\n");
    }
}

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

TemperatureLogger* TemperatureLoggerInit(
    const char *portName, int baudRate, const char *logFilePath, const char *hourlyLogFilePath, const char *dailyLogFilePath
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

void TemperatureLoggerClose(TemperatureLogger *logger) {
    if (logger) {
        SerialClose(logger->serialPort);
        free(logger);
    }
}

void LogTemperature(TemperatureLogger *logger, double temperature) {
    time_t now = time(NULL);
    struct tm *tmNow = localtime(&now);

    char logEntry[64];
    snprintf(logEntry, sizeof(logEntry), "%4d-%02d-%02d %02d:%02d:%02d %f",
             tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday,
             tmNow->tm_hour, tmNow->tm_min, tmNow->tm_sec, temperature);
    
    WriteToFile(logger->logFilePath, logEntry);
    WriteToDatabase((double)temperature);
}

void UpdateHourlyAverage(TemperatureLogger *logger, double* hourlySum, int *hourlyCount, time_t *lastHour) {
    if (*hourlyCount > 0) {
        time_t now = time(NULL);
        struct tm *tmNow = localtime(&now);

        char hourlyEntry[64];
        snprintf(hourlyEntry, sizeof(hourlyEntry), "%4d-%02d-%02d %02d:00 %f",
                 tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday,
                 tmNow->tm_hour, *hourlySum / *hourlyCount);
        WriteToFile(logger->hourlyLogFilePath, hourlyEntry);
    }
    
    *hourlySum = 0;
    *hourlyCount = 0;
    *lastHour = time(NULL);
}

void UpdateDailyAverage(TemperatureLogger *logger, double* dailySum, int* dailyCount, time_t* lastDay) {
    if (*dailyCount > 0) {
        time_t now = time(NULL);
        struct tm *tmNow = localtime(&now);

        char dailyEntry[64];
        snprintf(dailyEntry, sizeof(dailyEntry), "%4d-%02d-%02d %f",
                 tmNow->tm_year + 1900, tmNow->tm_mon + 1, tmNow->tm_mday,
                 *dailySum / *dailyCount);
        WriteToFile(logger->dailyLogFilePath, dailyEntry);
    }
    
    *dailySum = 0;
    *dailyCount = 0;
    *lastDay = time(NULL);
}

void ProcessTemperatureData(TemperatureLogger *logger, double temperature, double* hourlySum, int* hourlyCount, 
                                   double* dailySum, int* dailyCount, time_t* lastHour, time_t* lastDay) {
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

void TemperatureLoggerRun(TemperatureLogger *logger) {
    time_t lastHour = time(NULL);
    time_t lastDay = time(NULL);
    int hourlyCount = 0, dailyCount = 0;
    double hourlySum = 0, dailySum = 0;
    
    while (1) {
      char buffer[32];
      if (SerialRead(logger->serialPort, buffer, sizeof(buffer) - 1)) {
        char *end;
        double temperature = strtod(buffer, &end);
        printf("Считана температура из порта: %f\n", temperature);
        ProcessTemperatureData(logger, temperature, &hourlySum, &hourlyCount, &dailySum, &dailyCount, &lastHour, &lastDay);
      }
      SleepMs(1000);
    }
}