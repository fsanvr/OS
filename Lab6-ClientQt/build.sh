git pull origin main

mkdir build
cd build

qmake ../

make -j$(nproc)

./Lab6-ClientQt