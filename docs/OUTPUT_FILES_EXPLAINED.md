# Model Output Files
Topmodel source code generates output files `hyd.out` and `topmod.out` when `yes_print_output` (Boolean) is set to `TRUE`.
This toggle is the 3rd element (first line) in [`subcat.dat`](../data/subcat.dat).
- [Hydrograph Output](#hydrograph-output)
- [Model Results](#model-results)

The BMI extended version maintains this conditional logic within model code but also considers a larger scoped (framework-relevant) configurable equivalent via `stand_alone` switch.
See note on [STAND_ALONE](./STAND_ALONE.md).
I.e. when both `stand_alone` and `yes_print_output` are `TRUE` then model output files are generated.
Recall that `topmod.out` and `hyd.out` are defined in the last two lines in the primary configuration file, [`topmod.out`](../data/topmod.run), respectively.   

The header file [`topmodel.h`](../include/topmodel.h) also includes comments of each variable, both input and output.

## Hydrograph Output
A sample hydrograph output can be found in the source code directory; [`demo_hydrograph.out`](../refs/original_code_c/demo_hydrograph.out).
It contains observed data, `Qobs`, and simulated discharge,`Q`, as the image below details.

<p align="center">
  <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/hyd_out.PNG" width=40% height=40% >
</p>

## Model Results
See [`demo_topmod.out`](../refs/original_code_c/demo_topmod.out) for sample example model output.

<p align="center">
  <img src="https://github.com/madMatchstick/topmodel/blob/doc-update-bmi-v2.0/docs/img/topmod_out.PNG" width=75% height=75% >
</p>