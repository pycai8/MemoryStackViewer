#!/bin/bash

if [ ! -d output ]; then
    mkdir output
fi

if [ ! -d temp ]; then
    mkdir temp
fi

gcc -shared -fPIC -O0 -g3 MemoryStackViewer.c -o ./output/libmsv.so -ldl

g++ -O0 -g3 MemoryStackTranslate.cpp -o ./output/mst

g++ -O0 -g3 CreateThreadTest.cpp -o ./output/ctt -lpthread

g++ -O0 -g3 -c PerformanceTest.cpp -o temp/PerformanceTest.o

g++ -O0 -g3 temp/*.o  -lpthread -o ./output/pft

exit 0

