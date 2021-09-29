# Stand Alone Mode

A toggle or flag (Boolean) has been added to the model’s primary configuration file [`topmod.run`](../data/topmod.run) that indicates whether it will be run in `stand_alone` mode. 
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

Be aware the other input data files, `params.dat` and `subcat.dat` remain unaffected by this flag.

### Model Clock Time
Recall that the model run function `topmod()` is now devoid of a time-loop as outlined in [BMI_ADAPTION](./BMI_ADAPTION.md).
Under a `stand_alone` mode, the model will use a for-loop to step through time, making repeated calls to its BMI `update()` function.
The first two entries in `inputs.dat` define `nstep` and `dt`, respectively.

Otherwise, the model-coupling framework is in charge of a “clock” for tracking time, and knowing when, and how often, `update()` needs to be called.
The model’s own “clock” is bypassed, although the model must still keep track of its own “time” or “time_index” variable, just as it does in `stand_alone` mode.
The model-coupling framework can retrieve a model’s time information from a set of BMI functions that includes: `get_start_time()`, `get_current_time()`, `get_end_time()`, `get_time_step` and `get_time_units()`.

The behavior of model clock time, conditioned via `stand_alone`, is handled by `main.c`, a *pseudo-framework*, [here](../src/main.c#L42).
```  
  // Get number of timesteps
  topmodel_model *topmodel;
  topmodel = (topmodel_model *) model->data;
  int n;
  
  if (topmodel->stand_alone == TRUE){
    // Gather number of steps from input file
    // when in standalone mode.
    n = topmodel->nstep;
  }
  else{
    // Otherwise define loop here
    // Note: this is a pseudo-framework
    n = 720;
  }

  for (int i=1;i<=n;i++){
    model->update(model);
  }
```
  
## Outputs
`Qout` remains the main output variable for the model, regardless of `stand_alone` mode.
However, output files `topmod.out` and `hyd.out` are only considered when `stand_alone = TRUE`.
These outputs, as well as other end-of-run model statistics, are handled via `results()` within in BMI's `update()` found [here](../src/bmi_topmodel.c#L414)
```
if (topmodel->stand_alone == TRUE){
  results(topmodel->output_fptr, topmodel->out_hyd_fptr, topmodel->nstep, 
    topmodel->Qobs, topmodel->Q, 
    topmodel->current_time_step, topmodel->yes_print_output);
}
```

Once a model is BMI-enabled, its BMI model control functions will be used to run it whether or not it is in `stand_alone` mode.  
Simple if-then-else statements condition the model's behavior based on this toggle


