#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "logger/SerialPort.h"
#include "logger/TemperatureDeviceSimulator.h"
#include "logger/TemperatureLogger.h"

#include "database/Database.h"
#include "server/Server.h"
#include "config.h"


#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)

    #define WRITE_PORT "COM7"
    #define READ_PORT  "COM8"
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)

    #define WRITE_PORT "/dev/ttys004"
    #define READ_PORT  "/dev/ttys005"
#endif

#define BAUD_RATE       9600
#define LOG_FILE        "TemperatureLog.txt"
#define HOURLY_LOG_FILE "HourAvg.txt"
#define DAILY_LOG_FILE  "DayAvg.txt"


void *runTemperatureSimulator(void *arg) {
    TemperatureDeviceSimulator *simulator = (TemperatureDeviceSimulator *)arg;
    TemperatureDeviceSimulatorRun(simulator);
    return NULL;
}

void *runTemperatureLogger(void *arg) {
    TemperatureLogger* logger = (TemperatureLogger*)arg;
    TemperatureLoggerRun(logger);
    return NULL;
}


int main(int argc, char *argv[]) {
    printf("Запуск эмулятора температуры, логгера и сервера...\n");

    TemperatureDeviceSimulator* simulator = TemperatureDeviceSimulatorInit(
        WRITE_PORT,
        BAUD_RATE,
        -20.0,
        20.0,
        0.12,
        1000
    );

    if (!simulator) {
        fprintf(stderr, "Ошибка: не удалось инициализировать симулятор\n");
        return EXIT_FAILURE;
    }

    pthread_t simulatorThread;
    if (pthread_create(&simulatorThread, NULL, runTemperatureSimulator, simulator) != 0) {
        fprintf(stderr, "Ошибка: не удалось запустить поток симулятора\n");
        return EXIT_FAILURE;
    }

    TemperatureLogger* logger = TemperatureLoggerInit(
        READ_PORT,
        BAUD_RATE,
        LOG_FILE,
        HOURLY_LOG_FILE,
        DAILY_LOG_FILE
    );

    if (!logger) {
        fprintf(stderr, "Ошибка: не удалось инициализировать логгер\n");
        return EXIT_FAILURE;
    }

    pthread_t loggerThread;
    if (pthread_create(&loggerThread, NULL, runTemperatureLogger, logger) != 0) {
        fprintf(stderr, "Ошибка: не удалось запустить поток логгера\n");
        return EXIT_FAILURE;
    }

    if (!database_init(DatabaseFile)) {
        fprintf(stderr, "Ошибка: не удалось инициализировать БД\n");
        return EXIT_FAILURE;
    }

    if (!http_server_start(HttpUrl, PoolTimeoutMs)) {
        fprintf(stderr, "Ошибка: не удалось запустить HTTP-сервер\n");
        database_close();
        return EXIT_FAILURE;
    }

    pthread_join(simulatorThread, NULL);
    pthread_join(loggerThread, NULL);

    database_close();
    
    TemperatureDeviceSimulatorClose(simulator);
    TemperatureLoggerClose(logger);

    printf("Работа эмулятора температуры, логгера и сервера завершена.\n");

    return EXIT_SUCCESS;
}