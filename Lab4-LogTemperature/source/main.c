#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#include "SerialPort.h"
#include "TemperatureDeviceSimulator.h"
#include "TemperatureLogger.h"

#ifdef _WIN32
    #include <windows.h>
    #define SLEEP_MS(ms) Sleep(ms)

    #define WRITE_PORT "COM7"
    #define READ_PORT  "COM8"
#else
    #include <unistd.h>
    #define SLEEP_MS(ms) usleep((ms) * 1000)

    #define WRITE_PORT "/dev/ttyUSB0"
    #define READ_PORT  "/dev/ttyUSB1"
#endif

#define BAUD_RATE       9600
#define LOG_FILE        "TemperatureLog.txt"
#define HOURLY_LOG_FILE "HourAvg.txt"
#define DAILY_LOG_FILE  "DayAvg.txt"

/**
 * @brief Запуск симулятора термометра в отдельном потоке.
 * @param arg Указатель на структуру `TemperatureDeviceSimulator`
 * @return NULL
 */
void *runTemperatureSimulator(void *arg) {
    TemperatureDeviceSimulator *simulator = (TemperatureDeviceSimulator *)arg;
    TemperatureDeviceSimulatorRun(simulator);
    return NULL;
}

/**
 * @brief Функция для обработки времени работы, переданного через командную строку.
 * @param timeStr Время в формате "HH:MM".
 * @return Количество секунд, которое программа должна работать.
 */
int parseRunTime(const char *timeStr) {
    int hours, minutes;
    if (sscanf(timeStr, "%d:%d", &hours, &minutes) != 2) {
        fprintf(stderr, "Ошибка: Неверный формат времени. Ожидается HH:MM.\n");
        return -1;
    }
    return hours * 3600 + minutes * 60;
}

int main(int argc, char *argv[]) {
    printf("Запуск эмулятора температуры и логгера...\n");

    if (argc < 2) {
        fprintf(stderr, "Ошибка: Необходимо указать продолжительность работы программы в формате HH:MM.\n");
        return EXIT_FAILURE;
    }

    int runTime = parseRunTime(argv[1]);
    if (runTime < 0) {
        return EXIT_FAILURE;
    }

    time_t startTime = time(NULL);
    time_t endTime = startTime + runTime;

    TemperatureDeviceSimulator* simulator = TemperatureDeviceSimulatorInit(
        WRITE_PORT,
        BAUD_RATE,
        10,
        40,
        1000,
        endTime
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
        DAILY_LOG_FILE,
        endTime
    );

    if (!logger) {
        fprintf(stderr, "Ошибка: не удалось инициализировать логгер\n");
        return EXIT_FAILURE;
    }

    TemperatureLoggerRun(logger);
    TemperatureLoggerClose(logger);

    pthread_join(simulatorThread, NULL);
    TemperatureDeviceSimulatorClose(simulator);

    printf("Программа завершена после %d секунд работы.\n", runTime);

    return EXIT_SUCCESS;
}