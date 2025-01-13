#include <stdio.h>
#include "http_server.h"
#include "database.h"
#include "config.h"

int main() {
    if (!database_init(DatabaseFile)) {
        fprintf(stderr, "Ошибка: не удалось инициализировать БД\n");
        return 1;
    }

    if (!http_server_start(HttpUrl, PoolTimeoutMs)) {
        fprintf(stderr, "Ошибка: не удалось запустить HTTP-сервер\n");
        database_close();
        return 1;
    }

    database_close();
    return 0;
}