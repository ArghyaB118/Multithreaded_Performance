#!/bin/bash

g++ -std=c++11 -pthread ./sorting.cpp -o ./executables/sorting
#g++ -std=c++11 ./sorting.cpp -o ./executables/sorting
chmod a+x ./executables/sorting
g++ ./make-unsorted-data.cpp -o ./executables/make-unsorted-data
chmod a+x ./executables/make-unsorted-data

./executables/make-unsorted-data 256 data_files/nullbytes1
./executables/sorting 256 256 0 8 data_files/nullbytes1