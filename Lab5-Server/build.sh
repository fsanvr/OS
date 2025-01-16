#!/bin/bash

git pull origin main

qterminal -e "bash -c 'socat -d -d pty,raw,echo=0 pty,raw,echo=0; exec bash'" &

mkdir build
cd build

cmake ../

cmake --build .

./main