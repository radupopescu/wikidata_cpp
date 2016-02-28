#!/bin/sh

set -e

git clone https://github.com/zeromq/zmqpp.git
git clone https://github.com/zeromq/libzmq.git

cd libzmq
mkdir build
cd build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_INSTALL_PREFIX=$HOME/zmq \
    ../

make
make install

cd ../../

cd zmqpp
mkdir build
cd build
cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DCMAKE_PREFIX_PATH=$HOME/zmq \
    -DCMAKE_INSTALL_PREFIX=$HOME/zmq \
    -DZMQPP_BUILD_STATIC=OFF \
    ../

make
make install
