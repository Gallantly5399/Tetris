#!/bin/bash
cd $( dirname "$0" )
cmake -DCAMKE_BUILD_TYPE=Release -B build -S .
cmake --build build