#!/bin/bash
FILE=./run_ser_test
if test -f "$FILE"; then
    # If compilation fails, don't want to use old one
    rm "$FILE"
fi
# touch run_ser_test
# mv -f run_ser_test z_trash

gcc ./topmodel_serialize_test.c ./serialize_state.c ../src/bmi_topmodel.c ../src/topmodel.c -o run_ser_test -lm -lmsgpackc 
./run_ser_test
