git pull origin main

mkdir build
cd build

cmake ../

make -j$(nproc)

./main