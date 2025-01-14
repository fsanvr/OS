cmake_minimum_required(VERSION 3.10)

project(Lab5-Server C)

set(CMAKE_C_COMPILER "gcc")
set(CMAKE_C_STANDARD 11)

set(SOURCE_DIR ${CMAKE_SOURCE_DIR}/source)
set(LIB_DIR ${CMAKE_SOURCE_DIR}/lib)

set(SOURCES
    ${SOURCE_DIR}/main.c

    ${SOURCE_DIR}/logger/SerialPort.c
    ${SOURCE_DIR}/logger/TemperatureDeviceSimulator.c
    ${SOURCE_DIR}/logger/TemperatureLogger.c

    ${SOURCE_DIR}/database/Database.c
    ${SOURCE_DIR}/server/Server.c
)

set(LIB_SOURCES
    ${LIB_DIR}/sqlite3.c
    ${LIB_DIR}/mongoose.c
    ${LIB_DIR}/cJSON.c
)

include_directories(${SOURCE_DIR} ${LIB_DIR})

# Добавляем cJSON.c в исполняемый файл
add_executable(main ${SOURCES} ${LIB_SOURCES})

if(WIN32)
    target_link_libraries(main ws2_32)
elseif(UNIX)
    target_link_libraries(main pthread dl)
endif()
