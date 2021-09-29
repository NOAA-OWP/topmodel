# Model Output Files
Topmodel source code generates output files `hyd.out` and `topmod.out` when `yes_print_output` (Boolean) is set to `TRUE`.
This toggle is the the 3rd element (first line) in [`subcat.dat.`](../data/subcat.data)

The BMI extended version maintains this conditional logic within model code but also considers a larger scoped (framework-relevant) configurable equivalent via `stand_alone` switch.
See note on [STAND_ALONE](./STAND_ALONE.md).
I.e. when both `stand_alone` and `yes_print_output` are `TRUE` then model output files are generated.
Recall that `hyd.out` and `topmod.out` are defined in the last two lines in the primary configuration file, [`topmod.out`](../data/topmod.run), respectively.   

//Todo: add screenshots/tables of variables described for each.
