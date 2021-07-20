#!/bin/bash
touch run_bmi_test_unit
mv -f run_bmi_test_unit z_trash_test_unit
gcc ./main_test_unit_bmi.c ../src/topmodel.c ../src/bmi_topmodel.c -o run_bmi_test_unit -lm
./run_bmi_test_unit ./data/topmod_100.run