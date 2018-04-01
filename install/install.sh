#!/bin/bash

rm -r -f build
mkdir build
cd build
cmake ..
make
sudo make install
cd ..
sudo cp autocomplete/autolab /etc/bash_completion.d/
. /etc/bash_completion.d/autolab
