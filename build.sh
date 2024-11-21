#!/bin/bash
cd $( dirname "$0" ) || exit
cmake -DCMAKE_BUILD_TYPE=Release -B build -S .
cd build && cmake --build .