#!bin/bash
gcc -pthread -std=gnu99 -O2 -s pi.c -o pi
time ./pi 1 10000000000
time taskset -c 1,2 ./pi 2 10000000000
time taskset -c 1,3,5,7 ./pi 4 10000000000
