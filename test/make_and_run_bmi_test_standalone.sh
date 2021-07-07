#!/bin/bash
touch run_bmi_test_standalone
mv -f run_bmi_test_standalone z_trash_test_standalone
gcc ./main_test_standalone.c ../src/topmodel.c ../src/bmi_topmodel.c -o run_bmi_test_standalone -lm
./run_bmi_test_standalone