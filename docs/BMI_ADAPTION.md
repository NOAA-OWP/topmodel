# Adapting TOPMODEL to Basic Model Interface (BMI)

## Source Code 
-	TOPMODEL [C version](../references/1_ORG_CODE_C_FO/tmod9502.c) converted by Fred Ogden ‘09.
-	Previous is an adaptation from Keith Beven’s 1985 [FORTRAN version](../references/2_ORG_ORG_CODE_FORTRAN_KB/TMOD9502.f).

## Summary of Files
-	[`topmodel.c`](../src/topmodel.c) modified from orginal source
-	[`bmi_topmodel.c`](../src/bmi_topmodel.c) BMI functions
-	[`topmodel.h`](../include/topmodel.h) function/subroutine prototypes and model structure declaration
-	[`bmi_topmodel.h`](../include/bmi_topmodel.h) "core" bmi
-	[`bmi.h`](../include/bmi.h) "core" bmi
-	[`topmod.run`](../data/topmod.run) primary configuration file, listing input/output file paths
-	[`inputs.dat`](../data/inputs.dat) input data file (state dependent)
-	[`subcat.dat`](../data/subcat.dat) subcatchment topographic data file (incl. ln(a/tanB) dist.)
-	[`params.dat`](../data/params.dat) parameter based data file


## Summary of Changes
1.	Remove all `printf()` statements; nothing printed to console
2.	Use `yes_print_output` flag only for file print, `fprintf()`, throughout
3.	Separate variable declarations and function prototypes into header file `topmodel.h`
4.	Remove `main()` from `topmodel.c` & integrate into `bmi_topmodel.c`
	-	Variable declarations --> topmodel struct in `topmodel.h`
	-	File stream management --> `read_init_config()`
	-	Remove subcatment loop; to be handed by framework 
	-	Read-in functions --> `init_config()`
		-	`inputs()`: nstep, DT and RAINFALL, PE, QOBS from `inputs.dat`
		-	`fscanf()`: SUBCATCHMENT TOPOGRAPHIC DATA
		-	`tread()`: `subcat.dat`
		-	`init()`: MODEL PARAMETERS `params.dat`
	-	`topmod()` --> `update()`
	-	`results()` --> `update()`
	-	`fclose()` placed at end of each .dat read-in function
5.	Include ``current_time_step`` to `results()` and `topmod()`
6.	Remove timeloop from `topmod()`; set ``it`` to ``current_time_step``
7.	Include state variables in model structure, declared in `topmodel.h`, & to be passed in `topmod()`
	-	```sump```
	-	```sumae```
	-	```sumq```
8.	Only print summary results at end of model run `nstep==current_time_step`
9.  Note the use of `main()` in `bmi_topmodel.c` (or in separate .c file) acts as a *psuedo-framework* for standalone development  


