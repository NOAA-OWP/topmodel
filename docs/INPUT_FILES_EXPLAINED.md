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

| Variable | Datatype | Limits | Description |
| -------- | -------- | ------ | ----------- |
| stand_alone    | *int*    | Boolean | `1`(`TRUE`) or `0` (`FALSE`) flag to indicate if model is to be run via *stand alone* mode; added for BMI adaption |
| title          | *char*   | 256     | character title of model |
| \*input_fptr   | FILE   | 256     | file pointer to `inputs.DAT`  |
| \*subcat_fptr  | FILE   | 256     | file pointer to `subcat.DAT`  |
| \*params_fptr  | FILE   | 256     | file pointer to `params.DAT`  |
| \*output_fptr  | FILE   | 256     | file pointer to `topmod.OUT`  |
| \*out_hyd_fptr | FILE   | 256     | file pointer to `hyd.OUT`     |

Note: All file pointers must be a full relative path; e.g. `data/inputs.DAT`.

## Input Data
See [`inputs.dat`](../data/inputs.dat).

<p align="center">
  <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/inputs_dat.PNG" width=45% height=45% >
</p>

| Variable | Datatype | Units | Description |
| -------- | -------- | ----- | ----------- |
| nstep  | *int* |   | total number of simulation periods |
| dt     | *int* | hours  | size of timestep  |
| rain   | *double* |  meters/hour | rainfall rate  |
| pe     | *double* |  meters/hour | potential evapotranspiration  |
| Qobs   | *double* |  meters/hour | observed discharge   |

## Subcatchment Data
See [`subcat.dat`](../data/subcat.dat).

<p align="center">
  <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/subcat_dat.PNG" width=75% height=75% >
</p>

## Parameter Data
See [`params.dat`](../data/params.dat).

<p align="center">
  <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/params_dat.PNG" width=55% height=55% >
</p>

| Variable | Datatype | Limits | Units | Description |
| -------- | -------- | ------ | ----- | ----------- |
| subcat  | *char* | 256  |   | character title of subcatment; often same as model title  |
| szm     | *double* |   | meters | exponential scaling parameter for the decline of transmissivity with increase in storage deficit; units of depth  |
| t0   | *double* |   |  ln(meters^2) | areal average of ln(a/tanB)  |
| td   | *double* |   |  hours | unsaturated zone time delay per unit storage deficit  |
| chv  | *double* |   |  meters/hour | average channel flow velocity   |
| rv  | *double* |   |  meters/hour | internal overland flow routing velocity   |
| srmax  | *double* |   |  meters | maximum root zone storage deficit   |
| Q0  | *double* |   |  meters/hour | initial subsurface flow per unit area   |
| sr0  | *double* |   |  meters | initial root zone storage deficit below field capacity   |
| infex  | *int* | Boolean |   | set to `1` to call subroutine to do infiltration excess calcs; not usually appropriate in catchments where Topmodel is applicable (shallow highly permeable soils); default to `0` |
| xk0  | *double* |   |  meters/hour | surface soil hydraulic conductivity |
| hf | *double* |   |  meters | wetting front suction for G&A soln. |
| dth | *double* |   |   | water content change across the wetting front; dimensionless |