protoc --experimental_allow_proto3_optional -I=src --cpp_out=src proto/protocol.proto
mkdir -p build
cd build
export CXX=/usr/bin/clang++-14
export CC=/usr/bin/clang-14
cmake -DCMAKE_C_COMPILER=/usr/bin/clang-14 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-14 ..
make -j 8
