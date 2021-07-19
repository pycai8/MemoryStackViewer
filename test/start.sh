#!/bin/bash

gcc main.c -o test.exe
export LD_PRELOAD=../libMemoryStackViewer.so
./test.exe

exit 0

