cd ../
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DSANITIZE=Address
cmake --build build -j
cd build
./HftSimulatorTests