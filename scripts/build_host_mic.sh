#!/usr/bin/env bash

set -e 

if [ ! -d "x86_64" ]; then
  mkdir x86_64
  cd x86_64
  cmake ../../ -DINSTALL_PATH=lib
  cd ../
fi

if [ ! -d "k1om" ]; then
  mkdir k1om
  cd k1om
  cmake ../../ -DCMAKE_CXX_COMPILER=/usr/linux-k1om-4.7/bin/x86_64-k1om-linux-g++ -DCMAKE_C_COMPILER=/usr/linux-k1om-4.7/bin/x86_64-k1om-linux-gcc -DINSTALL_PATH=/usr/linux-k1om-4.7/linux-k1om/usr/lib64
  cd ../
fi

cd x86_64
make
sudo make install

cd ../k1om
make
sudo make install
cd ../
