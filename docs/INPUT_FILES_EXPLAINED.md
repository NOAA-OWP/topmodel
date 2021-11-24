# Model Input Files

- [Primary Configuration File](#primary-configuration-file)
- [Input Data](#input-data)
- [Subcatchment Data](#subcatchment-data)
- [Parameter Data](#parameter-data)

The header file [`topmodel.h`](../include/topmodel.h) also includes comments of each variable, both input and output.

## Primary Configuration File
See [`topmod.run`](../data/topmod.run) for Topmodel's main configuration file.
The image and table below explain the file's contents
All entry values can be customized, but must remain the same order for proper read-in during `bmi.initialize()`.
Note that the first `Boolean` `stand_alone` flag is specific to BMI extention and is not present in source code.
More on `stand_alone` can be found [here](./STAND_ALONE.md).

<p align="center">
  <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/topmod_run.PNG" width=45% height=45% >
</p>

| Variable | Datatype | Limits | Role | Description |
| -------- | -------- | ------ | ---- | ----------- |
| stand_alone | *int* | Boolean | option | `1`(`TRUE`) or `0` (`FALSE`) flag to indicate if model is to be run via *stand alone* mode; added for BMI adaption |
| title          | *char* | 256 | info_string | character title of model |
| \*input_fptr   | FILE   | 256 | file_offset | file pointer to `inputs.DAT`  |
| \*subcat_fptr  | FILE   | 256 | file_offset | file pointer to `subcat.DAT`  |
| \*params_fptr  | FILE   | 256 | file_offset | file pointer to `params.DAT`  |
| \*output_fptr  | FILE   | 256 | file_offset | file pointer to `topmod.OUT`  |
| \*out_hyd_fptr | FILE   | 256 | file_offset | file pointer to `hyd.OUT`     |

\*Note: All file pointers must be a full relative path; e.g. `data/inputs.DAT`.

## Input Data
See [`inputs.dat`](../data/inputs.dat).

<p align="center">
  <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/inputs_dat.PNG" width=45% height=45% >
</p>

| Variable | Datatype | Units | Role | Description |
| -------- | -------- | ----- | ---- | ----------- |
| nstep  | *int* |   | time_info | total number of simulation periods |
| dt     | *int* | hours  | time_info | size of timestep  |
| rain   | *double* |  meters/hour | input_from_file\* | rainfall rate  |
| pe     | *double* |  meters/hour | input_from_file\* | potential evapotranspiration  |
| Qobs   | *double* |  meters/hour | input_from_file\* | observed discharge   |

\*Note: Variable role is "input_from_bmi" if not in stand-alone mode.

## Subcatchment Data
See [`subcat.dat`](../data/subcat.dat).
\*Note: This file can be generated in workflow outlined [here](../params/README.md).

<p align="center">
  <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/subcat_dat.PNG" width=75% height=75% >
</p>

| Variable | Datatype | Limits | Units | Role | Process | Description |
| -------- | -------- | ------ | ----- | ---- | ------- | ----------- |
| num_sub_catchments | *int* |   |   | array_length |   | number of subcatments; BMI adaption always sets to 1 as loop to be handled by framework  |
| imap | *int* | Boolean  |   | option |   | ordinarily tells code to write map; NOT IMPLEMENTED |
| yes_print_output | *int*  | Boolean |   | option |   | set equal to `1` to print output files |
| subcat   | *char* | 256 |   | info_string |   | the name of each sub-catchment  |
| num_topodex_values  | *int* |   |   | parameter_fixed | rainfall-runoff | number of topodex histogram values |
| area  | *double* | 0-1 |  | parameter_fixed |   | catchment area as % to whole catchment (set to 1) |
| dist_area_lnaotb | *double* | 0-1 |  | parameter_fixed | rainfall-runoff | the distribution of area corresponding to ln(A/tanB) histo. |
| lnaotb | *double* |  |  | parameter_fixed | rainfall-runoff | ln(a/tanB) values; TWI |
| num_channels  | *int* |   |    | parameter_fixed | overland flow | number of channels |
| cum_dist_area_with_dist  | *double* | 0-1 |  | parameter_fixed | overland flow | channel cum. distr. of area with distance |
| dist_from_outlet | *double* |  | meters | parameter_fixed | overland flow | distance from outlet to point on channel with area known |

## Parameter Data
See [`params.dat`](../data/params.dat).

<p align="center">
  <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/params_dat.PNG" width=55% height=55% >
</p>

| Variable | Datatype | Limits | Units | Role | Process | Description |
| -------- | -------- | ------ | ----- | ---- | ------- | ----------- |
| subcat  | *char* | 256  |   | info_string |   | character title of subcatment; often same as model title  |
| szm     | *double* |   | meters | parameter_fixed | rainfall-runoff | exponential scaling parameter for the decline of transmissivity with increase in storage deficit; units of depth  |
| t0   | *double* |   |  ln(meters^2) | parameter_fixed |   | areal average of ln(a/tanB)  |
| td   | *double* |   |  hours | parameter_adjustable | rainfall-runoff | unsaturated zone time delay per unit storage deficit  |
| chv  | *double* |   |  meters/hour | parameter_fixed | overland flow | average channel flow velocity   |
| rv   | *double* |   |  meters/hour | parameter_fixed | overland flow | internal overland flow routing velocity   |
| srmax  | *double* |   |  meters | parameter_adjustable | rainfall-runoff | maximum root zone storage deficit   |
| Q0  | *double* |   |  meters/hour | state |   | initial subsurface flow per unit area   |
| sr0  | *double* |   |  meters | state |   | initial root zone storage deficit below field capacity   |
| infex  | *int* | Boolean |   | option | green-ampt | set to `1` to call subroutine to do infiltration excess calcs; not usually appropriate in catchments where Topmodel is applicable (shallow highly permeable soils); default to `0` |
| xk0  | *double* |   |  meters/hour | parameter_adjustable | rainfall-runoff | surface soil hydraulic conductivity |
| hf | *double* |   |  meters | parameter_adjustable | green-ampt |wetting front suction for G&A soln. |
| dth | *double* |   |   | parameter_adjustable | green-ampt | water content change across the wetting front; dimensionless |