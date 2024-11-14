# syntax=docker/dockerfile:1

FROM ubuntu:22.04
ENV DEBIAN_FRONTEND noninteractive

WORKDIR /5gsniffer
RUN apt -y update
RUN apt -y install git cmake make gcc g++ pkg-config libfftw3-dev libmbedtls-dev libsctp-dev libyaml-cpp-dev libgtest-dev libliquid-dev libconfig++-dev libzmq3-dev libspdlog-dev libfmt-dev libuhd-dev uhd-host clang
RUN git clone --recurse-submodules https://github.com/spritelab/5GSniffer.git .

WORKDIR /5gsniffer/5gsniffer
RUN mkdir -p build

WORKDIR /5gsniffer/5gsniffer/build
ENV CXX /usr/bin/clang++-14
ENV CC /usr/bin/clang-14
RUN cmake -DCMAKE_C_COMPILER=/usr/bin/clang-14 -DCMAKE_CXX_COMPILER=/usr/bin/clang++-14 ..
RUN make 

ENTRYPOINT ["/bin/bash"]