mkdir build
cd build
cmake .. -D CMAKE_BUILD_TYPE=Release -G "MinGW Makefiles"
mingw32-make VERBOSE=1
