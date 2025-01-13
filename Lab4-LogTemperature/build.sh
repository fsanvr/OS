git pull origin main

mkdir build
cd build

cmake ../

cmake --build .

./main "$@"