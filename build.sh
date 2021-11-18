#!/bin/bash

if [ ! -d output ]; then
    mkdir output
fi

gcc -shared -fPIC -O0 -g3 MemoryStackViewer.c -o ./output/libmsv.so -ldl

g++ -O0 -g3 MemoryStackTranslate.cpp -o ./output/mst

g++ -O0 -g3 CreateThreadTest.cpp -o ./output/ctt -lpthread

g++ -c PerformanceTest.cpp -o output/PerformanceTest.o

g++ -O0 -g3 output/*.o  -lpthread -o ./output/pft

exit 0

