@echo off
REM === Конфигурация ===
set BUILD_DIR=build
set REPO_URL=https://github.com/fsanvr/OS.git
set CMAKE_GENERATOR="MinGW Makefiles"
set CMAKE_COMPILER=g++

REM === Обновление исходных кодов ===
echo Updating source code from Git repository...
git pull %REPO_URL% 

REM === Создание build директории ===
IF NOT EXIST %BUILD_DIR% (
    mkdir %BUILD_DIR%
)

cd %BUILD_DIR%

REM === Настройка ===
echo Configuring project with CMake...
cmake -G %CMAKE_GENERATOR% ..
IF ERRORLEVEL 1 (
    echo CMake configuration failed!
    exit /b 1
)

REM === Компиляция ===
echo Building project...
cmake --build .
IF ERRORLEVEL 1 (
    echo Build failed!
    exit /b 1
)

REM === Завершение ===
echo Build completed successfully!
cd ..
exit /b 0

