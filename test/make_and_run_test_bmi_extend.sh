#!/bin/bash
touch run_bmi_test_extend
mv -f run_bmi_test_extend z_trash
gcc main_test_bmi_extend.c ../src/bmi_topmodel.c ../src/topmodel.c -o run_bmi_test_extend -lm
./run_bmi_test_extend
