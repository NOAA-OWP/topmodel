# BMI Unit Testing
Make and GCC are required for unit testing.
- [Component](#component)
- [Comparison](#comparison)
# Component 
We introduce several new components when extending computational models to BMI.  They are categorized as followed,  
- Model control functions (4)
- Model information functions (5)
- Variable information functions (6)
- Time functions (5)
- Variable getter and setter functions (5)
- Model grid functions (16)

We will fully examine functionality of all applicable definitions.

To run the BMI component test, 
```
$ cd alt-modular/Modules/TOPMODEL/test
$ make clean; make
$ ./run_unit_bmi_test ./data/topmod_100.run
```
The configuration file used here considers 100 timesteps of size 1 hour, but the actual testing loop is much smaller. 
To fully test the depth of all BMI functions, we are using a `stand_alone` value of `FALSE`.  See note on [`stand_alone`](#stand-alone-flag).
Recall that BMI guides interoperability for model-coupling, where model components (i.e. inputs and outputs) are easily shared amongst each other.
When testing outside of a true framework, we consider the behavior of BMI function definitions, rather than any expected values they produce. 

# Comparison
To compare bmi-enabled model results to output produced by original source code, we can run TOPMODEL in a `stand_alone` scenario. 
BMI model control functions (`initialize()`, `update()`, and `finalize()`) are specialized to manage this conditional switch. Other BMI functions primarily support use 
within a model-coupling framework.  Therefore, these “self-description” functions are unneeded for expected value tests.

```
$ cd alt-modular/Modules/TOPMODEL/src/
$ make clean; make
$ ./run_bmi
```
The executable is configured for a `stand_alone` scenario and reads-in information the same as the previous source code for TOPMODEL.  The two output files, `topmod.out` and `hyd.out`
are identical to those produced by the original source code, prior to any BMI implementation.  The results are a 1-1 match, as shown in the plot below.  Instructions on how to run original TOPMODEL can be
found [here](https://github.com/NOAA-OWP/alt-modular/blob/main/Modules/TOPMODEL/references/1_ORG_CODE_C_FO/README.1ST).

![image](https://user-images.githubusercontent.com/30940444/127345662-aa083fe7-d2d6-47a4-8c68-0a16686016c6.png)

## Stand Alone Flag
A Boolean toggle introduced to the main configuration file indicates whether the model will be run in a `stand_alone` setting.  
Note that BMI core control functions will always be used to run the model whether or not it is in `stand_alone` mode.
In this mode, the model will read required input data from files on its own and produce output files as seen from
original source code. 
