#!/bin/bash
touch run_bmi_test
mv -f run_bmi_test z_trash_test
gcc ./bmi_topmodel_test.c ../src/topmodel.c -o run_bmi_test -lm
./run_bmi_test