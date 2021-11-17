#!/bin/bash

gcc -shared -fPIC -O0 -g3 MemoryStackViewer.c -o libmsv.so -ldl

g++ -O0 -g3 MemoryStackTranslate.cpp -o mst

g++ -O0 -g3 CreateThreadTest.cpp -o ctt -lpthread

g++ -O0 -g3 PerformanceTest.cpp  -lpthread -o pft

exit 0

