#!/bin/bash

gcc -shared -fPIC -O3 -g3 MemoryStackViewer.c -o libmsv.so -ldl
strip libmsv.so

exit 0

