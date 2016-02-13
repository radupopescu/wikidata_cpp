#!/bin/sh

set -e

git clone https://github.com/zeromq/zmqpp.git
cd zmqpp

mkdir build
cd build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=$HOME/zmqpp \
    -DZMQPP_BUILD_STATIC=OFF \
    ../

make
make install
