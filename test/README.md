# BMI Unit Testing
Make and GCC compiler are required for unit testing.
- [Component](#component)
- [Comparison](#comparison)
- [Stand Alone Flag](#stand-alone-flag)

## Component 
We introduce several new components when extending computational models to BMI.  They are categorized as follows,  
- Model control functions (4)
- Model information functions (5)
- Variable information functions (6)
- Time functions (5)
- Variable getter and setter functions (5)
- Model grid functions (16)

We will fully examine the functionality of all applicable definitions.

To run the BMI component test, 
```
$ cd test/
$ make clean; make
$ ./run_bmi_unit_test
``` 
Note: Older compilers may require you to first run `module load gnu` prior to running `make`.

To fully test the depth of all BMI functions, we are using a `stand_alone` value of `FALSE` (`0`).  See note on [`stand_alone`](#stand-alone-flag).
Recall that BMI guides interoperability for model-coupling, where model components (i.e. inputs and outputs) are easily shared amongst each other.
When testing outside of a true framework, we consider the behavior of BMI function definitions, rather than any expected values they produce. 

## Comparison
To compare bmi-enabled model results to output produced by original source code, we can run TOPMODEL in a `stand_alone` scenario. 
BMI model control functions (`bmi.initialize()`, `bmi.update()`, and `bmi.finalize()`) are specialized to manage this conditional switch. 
Other BMI functions primarily support use within a model-coupling framework.
Therefore, these “self-description” functions are unneeded for expected value tests.

From the project's root directory,
```
$ cd src/
$ make clean; make
$ cd ..
$ ./run_bmi
```
The executable is configured for a `stand_alone` scenario and reads-in information the same as the previous source code for TOPMODEL.
The two output files, `topmod.out` and `hyd.out` are identical to those produced by the original source code, prior to any BMI implementation.
The results are a 1-1 match, as shown in the plots below.
Instructions on how to run original TOPMODEL can be found [here](../refs/original_code_c/README.1ST).
See [OUTPUT_FILES_EXPLAINED](../docs/OUTPUT_FILES_EXPLAINED.md) or [source code](../refs/original_code_c/tmod9502.c) for more information about output variable names and units.
The header file [`topmodel.h`](../include/topmodel.h) also includes comments of each variable, including both input and output.  

<img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/bmi_topmod_compare_hyd.png" width=50% height=50%> <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/bmi_topmod_compare_scatter.png" width=49% height=49%>

## Stand Alone Flag
A Boolean toggle introduced to the main [configuration file](./data/topmod_unit_test.run) indicates if the model will be run in a `stand_alone` setting.
The first line of the `topmod.run` file should be set to `1` (`TRUE`) if the model is to be configured for a stand-alone scenario and `0` (`FALSE`) if being run within a model-coupling framework.

Note that BMI core control functions will always be used to run the model whether or not it is in `stand_alone` mode.
In this mode, the model will read required input data from files on its own and produce output files as seen from original source code.

See [STAND_ALONE](../docs/STAND_ALONE.md) for more details. 
