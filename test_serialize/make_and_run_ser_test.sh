#!/bin/bash
FILE=./run_ser_test
if test -f "$FILE"; then
    # If compilation fails, don't want to use old one
    rm "$FILE"
fi

gcc ./topmodel_serialize_test.c ./serialize_state.c ../src/bmi_topmodel.c ../src/topmodel.c -o run_ser_test -lm -lmsgpackc 
cd ..
./test_serialize/run_ser_test
