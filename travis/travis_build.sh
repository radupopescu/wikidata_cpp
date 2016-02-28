#!/bin/sh

set -ev

cd $TRAVIS_BUILD_DIR
echo "Current directory" `pwd`

cd build-debug
cmake \
    -D CMAKE_BUILD_TYPE=Debug \
    -D ZEROMQ_INCLUDE_DIRS=$HOME/zmq/include \
    -D ZEROMQ_LIBRARY_DIRS=$HOME/zmq/lib \
    -D ZMQPP_INCLUDE_DIRS=$HOME/zmq/include \
    -D ZMQPP_LIBRARY_DIRS=$HOME/zmq/lib \
    ../
make

cd ../build-opt
cmake \
    -D CMAKE_BUILD_TYPE=Release \
    -D ZEROMQ_INCLUDE_DIRS=$HOME/zmq/include \
    -D ZEROMQ_LIBRARY_DIRS=$HOME/zmq/lib \
    -D ZMQPP_INCLUDE_DIRS=$HOME/zmq/include \
    -D ZMQPP_LIBRARY_DIRS=$HOME/zmq/lib \
    ../
make

