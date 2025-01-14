#include <stdio.h>
#include <stdlib.h>
#include "mongoose.h"
#include "cJSON.h"

#include "../database/Database.h"
#include "Server.h"

# define GET                    mg_str("GET")
# define POST                   mg_str("POST")

# define GetTemperatureLast     mg_str("/api/temperature/getlast")
# define GetTemperatureByDate   mg_str("/api/temperature/get")
# define AddTemperatureNew      mg_str("/api/temperature/set")

# define ResponceJsonHeader     "Content-Type: application/json\r\n"


static struct mg_mgr connectionManager;
static struct mg_connection *connections;


static void printMgString(struct mg_str str) {
    if (str.buf != NULL && str.len > 0) {
        printf("%.*s\n", (int)str.len, str.buf);
    }
}


static void logEvent(struct mg_http_message* message) {
    printf("Запрос (method, uri, query, proto): \n");
    printMgString(message->method);
    printMgString(message->uri);
    printMgString(message->query);
    printMgString(message->proto);
    printf("---------\n");
}


static int is_valid_date(const char *date) { //Format: YYYY-MM-DD
    int year, month, day;

    return sscanf(date, "%d-%d-%d", &year, &month, &day) == 3
        && month >= 1 && month <= 12
        && day >= 1 && day <= 31;
}


static char *SerializeTemperaturesToJson(TemperatureRecord *records, int count) {
    cJSON *root = cJSON_CreateArray();

    for (int i = 0; i < count; i++) {
        cJSON *entry = cJSON_CreateObject();
        cJSON_AddNumberToObject(entry, "timestamp", records[i].timestamp);
        cJSON_AddNumberToObject(entry, "temperature", records[i].temperature);
        cJSON_AddItemToArray(root, entry);
    }

    char *json_str = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);
    return json_str;  // Вызывающая сторона должна освободить память (free)
}


static void handleTemperatureGetLast(struct mg_connection *connection, struct mg_http_message* message) {

    int count;
    TemperatureRecord *records = database_get_last_temperature(&count);

    if (!records) {
        mg_http_reply(connection, 404, ResponceJsonHeader, "{\"error\":\"No data found\"}");
    }

    MG_INFO(("Responding with success"));
    char *json_response = SerializeTemperaturesToJson(records, count);
    MG_INFO(("Responding with success"));
    mg_http_reply(connection, 200, ResponceJsonHeader, json_response);
    free(json_response);
    free(records);
}


static void handleTemperatureGetByDates(struct mg_connection *connection, struct mg_http_message* message) {
    char startDate[20], endDate[20];

    mg_http_get_var(&message->query, "startDate", startDate, sizeof(startDate));
    mg_http_get_var(&message->query, "endDate", endDate, sizeof(endDate));

    if (startDate[0] == '\0' || endDate[0] == '\0') {
        mg_http_reply(connection, 400, ResponceJsonHeader, "Error: Missing required parameters: 'startDate' or 'endDate'\n");
        return;
    }

    if (!is_valid_date(startDate) || !is_valid_date(endDate)) {
        mg_http_reply(connection, 400, ResponceJsonHeader, "Error: Invalid format for 'startDate' or 'endDate'\n");
        return;
    }

    int count;
    TemperatureRecord *records = database_get_temperatures(startDate, endDate, &count);

    if (!records) {
        mg_http_reply(connection, 404, ResponceJsonHeader, "{\"error\":\"No data found\"}");
        return;
    }

    char *json_response = SerializeTemperaturesToJson(records, count);
    MG_INFO(("Responding with success"));
    mg_http_reply(connection, 200, ResponceJsonHeader, json_response);
    free(json_response);
    free(records);
}


static void handleTemperatureSet(struct mg_connection *connection, struct mg_http_message* message) {
    char temperatureString[20];
    double temperature;

    mg_http_get_var(&message->body, "temperature", temperatureString, sizeof(temperatureString));

    if (temperatureString[0] == '\0') {
        mg_http_reply(connection, 400, ResponceJsonHeader, "Error: Missing required parameters: 'temperature'\n");
        return;
    }

    char *endptr;
    temperature = strtod(temperatureString, &endptr);

    if (*endptr != '\0') {
        mg_http_reply(connection, 400, ResponceJsonHeader, "Error: Invalid format for 'temperature'\n");
        return;
    }

    if (!database_insert_temperature(temperature)) {
        mg_http_reply(connection, 400, ResponceJsonHeader, "Error: Couldn't make an entry\n");
        return;
    }

    MG_INFO(("Responding with success"));
    mg_http_reply(connection, 200, ResponceJsonHeader, "{\"status\":\"success\"}\n");
}


static void eventHandler(struct mg_connection *connection, int event, void *eventData) {
    if (event == MG_EV_HTTP_MSG) {
        struct mg_http_message *message = (struct mg_http_message *)eventData;

        logEvent(message);

        if (mg_match(message->method, GET, NULL)) {
            if (mg_match(message->uri, GetTemperatureLast, NULL)) {
                handleTemperatureGetLast(connection, message);
                return;
            }
            if (mg_match(message->uri, GetTemperatureByDate, NULL)) {
                handleTemperatureGetByDates(connection, message);
                return;
            }
        }
        if (mg_match(message->method, POST, NULL)) {
            if (mg_match(message->uri, AddTemperatureNew, NULL)) {
                handleTemperatureSet(connection, message);
                return;
            }
        }

        mg_http_reply(connection, 404, "Content-Type: text/plain\r\n", "Not found\n");
    }
}


bool http_server_start(const char *port, int poolTimeoutMs) {
    struct mg_mgr connectionManager;
    mg_mgr_init(&connectionManager);
    struct mg_connection *connections = mg_http_listen(&connectionManager, port, eventHandler, NULL);
    if (connections == NULL) {
        return false;
    }

    printf("Сервер запущен. Порт: %s\n", port);

    while (true) {
        mg_mgr_poll(&connectionManager, poolTimeoutMs);
    }

    mg_mgr_free(&connectionManager);
    return true;
}
