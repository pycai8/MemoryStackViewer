#!/bin/bash

gcc -shared -fPIC -O0 -g3 MemoryStackViewer.c -o libmsv.so -ldl

g++ -O0 -g3 MemoryStackTranslate.cpp -o mst

exit 0

