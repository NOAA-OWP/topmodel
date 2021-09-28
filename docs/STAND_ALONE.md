# Stand Alone Mode

A toggle or flag (Boolean) has been added to the model’s primiay configuration file [`topmod.run`](./data/topmod.run) that indicates whether it will be run in `stand_alone` mode. 
The first line should be set to `1` (`TRUE`) if the model is to be configured for a stand-alone scenario and `0` (`FALSE`) if being run within a model-coupling framework.

## Inputs
In `stand_alone` mode, the model will read required input data from files on its own, identical to the original source code.
[`inputs.dat`](../data/inputs.dat) contains forcing data, and inputs `nstep` and `dt`.
Specifically, `inputs()` from source code [`tmod9502.c`](../refs/original_code_c) reads in this data and allocates memory for output arrays accordingly. 
However, this function is only called when `stand_alone = TRUE`.
Otherwise, input values and memory allocation are initialized by model code but only for later management (via `set_value()`) by a framework driver.
Conditioning is handled within BMI's `initialize()` as seen in [this](../src/bmi_topmodel.c#L245) `if/else` block below.
```
if (model->stand_alone == TRUE){
  /* READ IN nstep, DT and RAINFALL, PE, QOBS INPUTS */
  inputs(model->input_fptr, &model->nstep, &model->dt, &model->rain, &model->pe, 
      &model->Qobs, &model->Q, &model->contrib_area);
  fclose(model->input_fptr);
}
else {

  /* Set nstep and dt*/
  model->nstep = 1;
  model->dt = 1;

  /* allocate memory for "arrays" */
  d_alloc(&model->rain,model->nstep);
  d_alloc(&model->pe,model->nstep);
  d_alloc(&model->Qobs,model->nstep);   
  d_alloc(&model->Q,model->nstep);
  d_alloc(&model->contrib_area,model->nstep);

  (model->rain)[1]=0.0;
  (model->pe)[1]=0.0;
  (model->Qobs)[1]=0.0;
  (model->Q)[1]=0.0;
  (model->contrib_area)[1]=0.0;
}
```
  
## Outputs
`Qout` remains the main output varible for the model, regardless of `stand_alone` mode.
However, output files, `topmod.out` and `hyd.out` are only considered when `stand_alone = TRUE`.
```
if (topmodel->stand_alone == TRUE){
  results(topmodel->output_fptr, topmodel->out_hyd_fptr, topmodel->nstep, 
    topmodel->Qobs, topmodel->Q, 
    topmodel->current_time_step, topmodel->yes_print_output);
}
```

Once a model is BMI-enabled, its BMI model control functions will be used to run it whether or not it is in `stand_alone` mode.  
Simple if-then-else statements condition the model's behavior based on this toggle
