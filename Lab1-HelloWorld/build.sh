#!/bin/bash

# === Конфигурация ===
BUILD_DIR="build"
REPO_URL="https://github.com/fsanvr/OS.git"
CMAKE_GENERATOR="Unix Makefiles"
CMAKE_COMPILER="g++"

# === Обновление исходных кодов ===
echo "Updating source code from Git repository..."
git pull %REPO_URL% 

# === Создание build директории ===
if [ ! -d "$BUILD_DIR" ]; then
    mkdir -p "$BUILD_DIR"
fi

cd "$BUILD_DIR" || exit

# === Настройка ===
echo "Configuring project with CMake..."
cmake -G "$CMAKE_GENERATOR"
if [ $? -ne 0 ]; then
    echo "CMake configuration failed!"
    exit 1
fi

# === Компиляция ===
echo "Building project..."
cmake --build .
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# === Завершение сборки ===
echo "Build completed successfully!"
cd ..
exit 0
