#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/topmodel.h"
#include "../include/bmi.h"
#include "../include/bmi_topmodel.h"

/*
This main program is a mock framwork.
This is not part of BMI, but acts as the driver that calls the model.
Note there are no getter/setter functions here,
therefore, when stand_alone FALSE, forcings/et data not being passed to it's array
and the inital NULL value is being maintained throughout model run  
*/


int main(void)
{
  
  // allocating memory to store the entire BMI structure
  Bmi *model = (Bmi *) malloc(sizeof(Bmi));

  // register bmi topmodel
  register_bmi_topmodel(model);

  // set config
  const char *cfg_file = "./data/topmod_nstep.run";

  printf("\n Initializeing topmodel bmi model ... \n");
  model->initialize(model, cfg_file);
  
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
    n = 100;
  }
  
  printf(" topmodel->nstep: %i\n", topmodel->nstep);
  printf(" n: %i\n", n);

  // Add ET data from TOPMODEL forcing
  double et_dbl[] = {0.0000619,0.0000647,0.0000619,0.000056,0.0000457,0.0000323,0.0000156,0,0,0,0,0,0,0,0,0,0,0,0,0,0.000018,0.0000348,0.0000492,0.0000602,0.0000666,0.0000696,0.0000666,0.0000602,0.0000492,0.0000348,0.0000168,0,0,0,0,0,0,0,0,0,0,0,0,0,0.0000338,0.0000652,0.0000923,0.000113,0.000125,0.0001305,0.000125,0.000113,0.0000923,0.0000652,0.0000316,0,0,0,0,0,0,0,0,0,0,0,0,0,0.0000414,0.0000798,0.0001129,0.0001383,0.000153,0.0001598,0.000153,0.0001383,0.0001129,0.0000798,0.0000387,0,0,0,0,0,0,0,0,0,0,0,0,0,0.0000191,0.0000368,0.0000521,0.0000638,0.0000706,0.0000737,0.0000706,0.0000638};

  // Add precip data from TOPMODEL forcing
  double ppt_dbl[] = {0,0,0.001,0.001,0.001,0.0005,0.0005,0,0.0005,0.0005,0,0,0.001,0.0005,0.0005,0.001,0,0,0,0,0,0,0,0.003,0.002,0.0055,0.006,0.001,0.0025,0,0,0.001,0.0005,0,0,0,0,0,0,0,0,0,0,0,0,0.001,0,0.0005,0,0.001,0.0025,0.003,0.0025,0.003,0.0025,0.002,0.001,0.001,0,0.0005,0.0005,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0.0005,0.0005,0,0};

  // Set up var and var name for using BMI to set PET and PPT
  double *var_pet = NULL;
  var_pet = (double*) malloc (sizeof (double)*1);
  const char *var_pet_name = "water_potential_evaporation_flux";

  double *var_ppt = NULL;
  var_ppt = (double*) malloc (sizeof (double)*1);
  const char *var_ppt_name = "atmosphere_water__liquid_equivalent_precipitation_rate";

  double current_time;
  for (int i=0;i<n;i++){
      
    if (topmodel->stand_alone == FALSE){
      //Get values from arrays and set_value()s
      var_pet[0] = et_dbl[i];
      var_ppt[0] = ppt_dbl[i];
      //Note: comment these out to maintain NULL value for testing
      model->set_value(model, var_pet_name, &(var_pet[0]));
      model->set_value(model, var_ppt_name, &(var_ppt[0]));
    }
    
    model->update(model);
    model->get_current_time(model, &current_time);
    printf("\n Updating ... current time: %f\n",current_time);

    //print some values
    printf("    topmodel-> Qout: %6.4e\n", topmodel-> Qout);
    printf("    topmodel-> p: %6.4e\n", topmodel-> p);
    printf("    topmodel-> ep: %6.4e\n", topmodel-> ep);
    printf("    topmodel-> Q: %6.4e\n", topmodel-> Q);
    printf("    topmodel-> quz: %6.4e\n", topmodel-> quz);
    printf("    topmodel-> qb: %6.4e\n", topmodel-> qb);
    printf("    topmodel-> sbar: %6.4e\n", topmodel-> sbar);
    printf("    topmodel-> qof: %6.4e\n", topmodel-> qof);

  }

  printf("\n Finalizing topdmodel bmi model ... \n");
  model->finalize(model);

  return 0;
}
