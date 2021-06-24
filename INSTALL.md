# Project Dependencies

## C Compiler

At present, [GCC](https://gcc.gnu.org/) is officially supported.

Tested version (GCC) 8.3.1

## OS

TOPMODEL is compatible on both Windows and Linux machines. 

Tested version RHEL v8.3.1-5

## Make

Recommended build tool [GNU Make](https://www.gnu.org/software/make/)

# Build and Run

Run TOPMODEL BMI using Make, 
```sh
cd src/
make clean; make
cd ..
./run_bmi
```

Default configuration generates OUT files, topmod.out and hyd.out, to root directory.
See [here](refs/original_code_c/demo_hydrograph.out) for example of hydrograph output..