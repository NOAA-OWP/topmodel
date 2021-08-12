#!/bin/bash
touch run_bmi_test_nstep
mv -f run_bmi_test_nstep z_trash
gcc main_test_nstep.c ../src/bmi_topmodel.c ../src/topmodel.c -o run_bmi_test_nstep -lm
./run_bmi_test_nstep