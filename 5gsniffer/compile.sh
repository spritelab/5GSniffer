mkdir -p build
cd build
export CXX=/usr/bin/clang++-12
export CC=/usr/bin/clang-12
#export CMAKE_CXX_COMPILER=/usr/bin/clang++
cmake -DCMAKE_C_COMPILER=/usr/bin/clang-12 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-12 ..
make -j 8
