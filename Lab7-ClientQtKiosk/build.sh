#!/bin/bash

git pull origin main

mkdir build
cd build

qmake ../

make -j$(nproc)

./Lab7-ClientQtKiosk