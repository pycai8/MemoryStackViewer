#!/bin/bash

gcc main.c -o test
export LD_PRELOAD=../libMemoryStackViewer.so
./test

exit 0

