#ifndef DATABASE_H
#define DATABASE_H

#include <stdbool.h>


typedef struct {
    int timestamp;
    double temperature;
} TemperatureRecord;

bool database_init(const char *db_path);

void database_close();

bool database_insert_temperature(double temperature);

TemperatureRecord* database_get_last_temperature(int *count);

TemperatureRecord* database_get_temperatures(const char *day_start, const char *day_end, int *count);


#endif  // DATABASE_H
