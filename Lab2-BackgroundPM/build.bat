git pull origin main

mkdir build
cd build

cmake .. -G "MinGW Makefiles"

cmake --build .

HelloWorld.exe

cd ..