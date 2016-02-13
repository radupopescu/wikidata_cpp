#!/bin/sh

set -ev

cd $TRAVIS_BUILD_DIR
echo "Current directory" `pwd`

cd build-debug
cmake \
    -D CMAKE_BUILD_TYPE=Debug \
    -D ZMQPP_INCLUDE_DIRS=$HOME/zmqpp/include \
    -D ZMQPP_LIBRARY_DIRS=$HOME/zmqpp/lib \
    ../
make

cd ../build-opt
cmake \
    -D CMAKE_BUILD_TYPE=Release \
    -D ZMQPP_INCLUDE_DIRS=$HOME/zmqpp/include \
    -D ZMQPP_LIBRARY_DIRS=$HOME/zmqpp/lib \
    ../
make

