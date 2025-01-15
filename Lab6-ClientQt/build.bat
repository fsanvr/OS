git pull origin main

mkdir build
cd build

cmake .. -G "MinGW Makefiles" > log_make.txt 2>&1

cmake --build . > log_build.txt 2>&1

main.exe

cd ..