#!/bin/bash

gcc -shared -fPIC -O0 -g3 MemoryStackViewer.c -o libMemoryStackViewer.so -ldl

exit 0

