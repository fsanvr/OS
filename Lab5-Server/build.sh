git pull origin main

socat -d -d pty,raw,echo=0 pty,raw,echo=0

mkdir build
cd build

cmake ../

cmake --build .

./main