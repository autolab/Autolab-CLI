#!/bin/bash

rm -r -f build
mkdir build
cd build
cmake .. -Drelease=ON
make
cd ..

