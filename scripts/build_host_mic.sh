#!/usr/bin/env bash

set -e

MY_CMAKE_FLAGS="-DCMAKE_BUILD_TYPE=Debug -DCMAKE_CXX_COMPILER=icpc -DCMAKE_C_COMPILER=icc"

if [ ! -d "x86_64" ]; then
  mkdir x86_64
fi

cd x86_64
#cmake ../../ -DINSTALL_PATH=lib
#cmake ../../ -DCMAKE_CXX_COMPILER=icpc -DCMAKE_CXX_LINK_EXECUTABLE=icpc -DCMAKE_C_LINK_EXECUTABLE=icc -DCMAKE_C_COMPILER=icc -DINSTALL_PATH=lib
cmake ../../ ${MY_CMAKE_FLAGS} -DINSTALL_PATH=lib
cd ../

if [ ! -d "k1om" ]; then
  mkdir k1om
fi

cd k1om
#cmake ../../ -DCMAKE_CXX_COMPILER=/usr/linux-k1om-4.7/bin/x86_64-k1om-linux-g++ -DCMAKE_C_COMPILER=/usr/linux-k1om-4.7/bin/x86_64-k1om-linux-gcc -DINSTALL_PATH=/usr/linux-k1om-4.7/linux-k1om/usr/lib64
#env CFLAGS="-mmic" CXXFLAGS="-mmic" cmake ../../ -DCMAKE_CXX_LINK_EXECUTABLE=icpc -DCMAKE_C_LINK_EXECUTABLE=icc -DCMAKE_CXX_COMPILER=icpc -DCMAKE_C_COMPILER=icc -DINSTALL_PATH=/usr/linux-k1om-4.7/linux-k1om/usr/lib64
env CFLAGS="-mmic" CXXFLAGS="-mmic" cmake ../../ ${MY_CMAKE_FLAGS} -DINSTALL_PATH=/usr/linux-k1om-4.7/linux-k1om/usr/lib64
cd ../

cd x86_64
make 
sudo make install

cd ../k1om
make
sudo make install
cd ../
