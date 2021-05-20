#!/bin/bash
touch run_bmi
mv -f run_bmi z_trash
gcc ./src/main.c ./src/bmi_topmodel.c ./src/topmodel.c -o run_bmi -lm
./run_bmi