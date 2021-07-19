#!/bin/bash

gcc *.c -g3 -shared -fPIC -o libMemoryStackViewer.so -lpthread -ldl

exit 0

