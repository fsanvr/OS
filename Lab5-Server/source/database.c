#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include "sqlite3.h"

#include "database.h"

static sqlite3 *db;


bool database_init(const char *db_path) {
    if (sqlite3_open(db_path, &db) != SQLITE_OK) {
        fprintf(stderr, "Ошибка открытия БД: %s\n", sqlite3_errmsg(db));
        return false;
    }

    const char *sql =
        "CREATE TABLE IF NOT EXISTS temperature_log ("
        "id INTEGER PRIMARY KEY AUTOINCREMENT, "
        "timestamp INTEGER DEFAULT (strftime('%s', 'now')), "
        "temperature REAL NOT NULL);";
    
    return sqlite3_exec(db, sql, NULL, NULL, NULL) == SQLITE_OK;
}


void database_close() {
    sqlite3_close(db);
}


bool database_insert_temperature(double temperature) {
    const char *sql = "INSERT INTO temperature_log (temperature) VALUES (?);";
    sqlite3_stmt *stmt;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return false;
    }

    sqlite3_bind_double(stmt, 1, temperature);
    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}


TemperatureRecord* database_get_last_temperature(int *count) {
    const char *sql = "SELECT timestamp, temperature FROM temperature_log ORDER BY id DESC LIMIT 1;";
    sqlite3_stmt *stmt;
    TemperatureRecord *record = NULL;
    *count = 0;

    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) {
            record = malloc(sizeof(TemperatureRecord));
            if (record) {
                record->timestamp = sqlite3_column_int(stmt, 0);
                record->temperature = sqlite3_column_double(stmt, 1);
                *count = 1;
            }
        }
    }
    sqlite3_finalize(stmt);
    return record;
}


TemperatureRecord* database_get_temperatures(const char *day_start, const char *day_end, int *count) {
    const char *sql =
        "SELECT timestamp, temperature FROM temperature_log "
        "WHERE timestamp >= strftime('%s', ? || ' 00:00:00') "
        "AND timestamp <= strftime('%s', ? || ' 23:59:59') "
        "ORDER BY timestamp ASC;";

    sqlite3_stmt *stmt;
    *count = 0;
    
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, NULL) != SQLITE_OK) {
        return NULL;
    }

    sqlite3_bind_text(stmt, 1, day_start, -1, SQLITE_STATIC);
    sqlite3_bind_text(stmt, 2, day_end, -1, SQLITE_STATIC);


    while (sqlite3_step(stmt) == SQLITE_ROW) {
        (*count)++;
    }

    if (*count == 0) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    TemperatureRecord *records = malloc((*count) * sizeof(TemperatureRecord));
    if (!records) {
        sqlite3_finalize(stmt);
        return NULL;
    }

    sqlite3_reset(stmt);
    *count = 0;


    while (sqlite3_step(stmt) == SQLITE_ROW) {
        records[*count].timestamp = sqlite3_column_int(stmt, 0);
        records[*count].temperature = sqlite3_column_double(stmt, 1);
        (*count)++;
    }

    sqlite3_finalize(stmt);
    return records;
}