# Adapting TOPMODEL to Basic Model Interface (BMI)
- [Source Code](#source-code)
- [Listing of Files](#listing-of-files)
- [Summary of Functions and Workflow](#summary-of-functions-and-workflow)
- [Summary of Changes](#summary-of-changes)
	- [Include Header File](#include-header-file)
	- [File Stream Management](#file-stream-management)
	- [Console Prints](#console-prints)
	- [`topmod` Function](#topmod-function)
	- [`results` Function](#results-function)
	- [`water_balance` Function](#water_balance-function)

## Source Code 
-	TOPMODEL [C version](../refs/original_code_c/tmod9502.c) converted by Fred Ogden ‘09.
-	Previous is an adaptation from Keith Beven’s 1985 [FORTRAN version](../refs/original_code_fortran/TMOD9502.f).

## Listing of Files
-	[`topmodel.c`](../src/topmodel.c): Modified from orginal source
-	[`bmi_topmodel.c`](../src/bmi_topmodel.c): Core BMI v2.0 functions defined by developer
-	[`main.c`](../src/main.c): *Pseudo-framework* for standalone BMI development and testing, calling model control functions
-	[`topmodel.h`](../include/topmodel.h): Function/subroutine prototypes and model structure declaration
-	[`bmi_topmodel.h`](../include/bmi_topmodel.h): Core BMI v2.0 
-	[`bmi.h`](../include/bmi.h): Core BMI v2.0; function prototypes 
-	[`topmod.run`](../data/topmod.run): Primary configuration file, listing input/output file paths
-	[`inputs.dat`](../data/inputs.dat): Input data file (forcings and time information)
-	[`subcat.dat`](../data/subcat.dat): Subcatchment topographic data file (includes ln(a/tanB) dist.)
-	[`params.dat`](../data/params.dat): Parameter based data file
-	[`topmod.out`](../data/topmod.out): (Optional) model results file
-	[`hyd.out`](../data/params.dat): (Optional) hydrograph output file

## Summary of Functions and Workflow
This nestest listing aims to demonstrate general flow of essential functions calls; i.e. where and when they are used under a mock `main.c` program or true framework setting.
E.g. "3. `bmi.update()`", calls "3.i. `topmod()`", which calls "3.i.a. `expinf()`", and also "3.ii. `results()`".
Some functions are native to core BMI v2.0, developer-defined additions to BMI domain space, or pulled from source code.

1. `bmi.register_bmi_topmodel()`
2. `bmi.initialize()`
	1. `bmi.init_config()`
		1. `bmi.read_init_config()`
		2. `inputs()`
		3. `tread()`
		4. `init()`
3. `bmi.update()`
	1. `topmod()`
		1. `expinf()`
	2. `results()`
4. `bmi.finalize()`
	1. `water_balance()`

In the following table, see "Notes" column for adaption adjustment details.
If "yes", columns "topmod9502.c?" and "topmodel-bmi?" provide links to location of function *definition*, not where it is called.   
All BMI functions are thoroughly documented by [CSDMS](https://bmi.readthedocs.io/en/latest/).  

| Function | Purpose | tmod9502.c? | topmodel-bmi? | Notes |
| -------- | ------- | ----------- | ------------- | ----- |
| `main()` | *Psuedo-framework* for building and running model control functions | [yes](../refs/original_code_c/tmod9502.c#L151) | [yes](../src/main.c#17) | Completely "absorbed" into BMI functions as explained in this document |
| `bmi.register_bmi_topmodel()` | allocates memory and generates BMI object class | no | [yes](../src/bmi_topmodel.c#L1044) | Core BMI v2.0; See [Header File](#include-header-file) |
| `bmi.initialize()` | accepts configuration file path and completes other initial tasks | no | [yes](../src/bmi_topmodel.c#L352) | Core BMI v2.0 |
| `bmi.init_config()` | reads-in data for all input files | no | [yes](../src/bmi_topmodel.c#L240) | Developer-defined to BMI space; also see [INPUT_FILES_EXPLAINED](./INPUT-FILES_EXPLAINED.md) |
| `bmi.read_init_config()` | reads-in info from configuration and sets appropriate file handles and pointers | no* | [yes](../src/bmi_topmodel.c#L142) | Developer-defined to BMI space; *encompasses lines 231-254 in [source code](../refs/original_code_c/tmod9502.c#L231) |
| `inputs()` | reads-in data from file `inputs.dat` (forcings and time info) | [yes](../refs/original_code_c/tmod9502.c#L548) | [yes](../src/topmodel.c#L406) | Function definition identical to source code |
| `tread()` | reads-in data from file `subcat.dat` (topographic index distribution and channel info) | [yes](../refs/original_code_c/tmod9502.c#L275) | [yes](../src/topmodel.c#L444) | Function definition identical to source code |
| `init()` | reads-in parameter data from file `params.dat` | [yes](../refs/original_code_c/tmod9502.c#L666) | [yes](../src/bmi_topmodel.c#L281) | Function definition identical to source code |
| `bmi.update()` | advances the model by a single time step | no | [yes](../src/bmi_topmodel.c#L375) | Core BMI v2.0 |
| `topmod()` | model run function | [yes](../refs/original_code_c/tmod9502.c#L288) | [yes](../src/topmodel.c#L542) | See [`topmod` Function](#topmod-function) for full details |
| `expinf()` | calculates infiltration excess runoff via Green-ampt routine | [yes](../refs/original_code_c/tmod9502.c#L817) | [yes](../src/topmodel.c#L698) | Function definition identical to source code |
| `results()` | prints model results to console/file(s) and calculations objective functions | [yes](../refs/original_code_c/tmod9502.c#L459) | [yes](../src/topmodel.c#L388) | See [`results` Function](#results-function) for full details |
| `bmi.finalize()` | deallocates memory, closes all files | no | [yes](../src/bmi_topmodel.c#L450) | Core BMI v2.0 |
| `water_balance()` | calculates water balance statistics | no | [yes](../src/bmi_topmodel.c#L377) | Not a part of BMI but added to `topmodel.c` for convenience; see [`water_balance`](#water_balance-function) for full details |

## Summary of Changes
Alterations to source code were necessary for Topmodel BMI extension.
However, the development was pursued in such a manner that minimum adjustments were made and moreso, that model code would be recognizable by its original author.   

### Include Header File
All variable declarations and function prototypes (lines 153-222 in [source code](../refs/original_code_c/tmod9502.c#L153)) were separated into a header file `topmodel.h` for convenience.
Variable array pointers ([source code](../refs/original_code_c/tmod9502.c#220) lines 200-212) are now initialized to `NULL` in `bmi.register_bmi_topmodel()`.
Maximum values for array allocation (lines 225-227 in [source code](../refs/original_code_c/tmod9502.c#225)) set in `bmi.initialize()`.

Additional variables needed for BMI,
```      
/******************* BMI vars ********************/ 
int current_time_step;    /*current time step*/
                        /*for BMI time functions*/

/***************** State Var Sums ****************/
double sump;   /* accumulated rainfall */
double sumae;  /* accumulated evapotranspiration */   
double sumq;   /* accumulated discharge (Qout) */
double sumrz;  /* deficit_root_zone over the whole watershed */
double sumuz;  /* stor_unsat_zone over the whole watershed */

/************** Additional Output vars **************/ 
double quz; /* flow from root zone to unsaturated zone*/
double qb;  /* subsurface flow or baseflow*/
double qof; /* flow from saturated area and infiltration excess flow*/
double p;   /* adjusted rain*/
double ep;  /* adjusted potential evaporation*/

/************** Framework vars **************/ 
int stand_alone;
```  
Note that source code *state variable sums* and *additional output variables* are now declared as *pointers* to be passed among various functions outside of the model's primary run function, `topmod()`.
More on `stand_alone` can be found [here](./STAND_ALONE.md).   

Also, variable `isc` ([source code](../refs/original_code_c/tmod9502.c#212)) was removed during BMI extension, as the subcatchment loop is no longer handled via `topmod()` but by a model-coupling framework, exterior to this model. 

### File Stream Management	
-	File pointers and handles (lines 231-254 in [source code](../refs/original_code_c/tmod9502.c#L231)) now managed via `bmi.read_init_config()`
-	`bmi.init_config` reads-in, allocates memory and stores all inputs-from-file data (or simply allocates and initiates otherwise)
-	All input files (`inputs.dat`, `subcat.dat` and `params.dat`), if opened, are closed during `bmi.initialize()`
- 	All output files (`topmod.out` and `hyd.out`), if opened, are closed during `bmi.finalize()` 

### Console Prints
`printf()` statements now handled by `TOPMODEL_DEBUG` found in [`topmodel.h`](../include/topmodel.h#L18).  Note: Macro uses greater than or equal to logic `>=`; i.e. a setting of `2` will include `0`, `1`, & `2`. Verbosity levels are, 

	-	0: Nothing
	-	1: Error messages (& exit program) 
	-	2: Model info (from source code)
	-	3: BMI info (e.g. current timestep)	

### topmod Function
-	Include ``current_time_step``  to `topmod()`.
-	Remove time-loop from `topmod()`; set ``it`` to ``current_time_step``.
-	Include state variables in model structure, declared in `topmodel.h`, & to be passed in `topmod()`
	-	```sump```
	-	```sumae```
	-	```sumq```

###	results Function
-	Include ``current_time_step`` to `results()`.
-	Only print summary results at end of model run `nstep==current_time_step`.

###	water_balance Function