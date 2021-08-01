#!/bin/bash

export LD_PRELOAD=../libMemoryStackViewer.so
./test.exe
ls
exit 0

