#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/topmodel.h"
#include "../include/bmi.h"
#include "../include/bmi_topmodel.h"


/*
This main program is a mock framwork.
This is not part of BMI, but acts as the driver that calls the model.
When stand_alone TRUE, inputs-forcings data passed to it's array 
via inputs.dat (standard to native code);
otherwise use get/set values or maintain inital NULL value (mimick framework)
*/

int main(void)
{
  
  // allocating memory to store the entire BMI structure
  //printf("\n Allocating memory to TOPMODEL BMI model structure ... \n");
  Bmi *model = (Bmi *) malloc(sizeof(Bmi));

  //printf("\n Registering TOPMODEL BMI model ... \n");
  register_bmi_topmodel(model);

  topmodel_model *topmodel;
  topmodel = (topmodel_model *) model->data;

  const char *cfg_file = "./data/topmod_100.run";
  //const char *cfg_file = "./data/topmod.run";
 
  //printf("\n Initializeing TOPMODEL BMI model ... \n");
  model->initialize(model, cfg_file);

  //Get number of timesteps
  double current_time, end_time, time_step;
  model->get_end_time(model, &end_time);
  model->get_time_step(model, &time_step);
  int numsteps = end_time/time_step;
  //printf(" \n nstep (S/F switch applied): %d\n", numsteps);

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

  //printf("\n Looping Update TOPMODEL BMI model\n");
  for (int i=0;i<numsteps;i++){

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
    
    printf("\n Current time hrs: %f\n",current_time);
    if (topmodel->stand_alone == FALSE){
      printf("    INPUTS:\n");
      //printf("      PET value from ET array %8.6e\n", et_dbl[i]); 
      printf("      PET from TOPMODEL struct %8.6e\n", topmodel->potential_et_m_per_s);
      //printf("      PET from ORIG struct %8.6e\n", topmodel->pe);
      //printf("      PPT value from PPT array %8.6e\n", ppt_dbl[i]);
      printf("      PPT from TOPMODEL struct %8.6e\n", topmodel->precip_rate);
      //printf("      PPT from ORIG struct %8.6e\n", topmodel->rain);
    }
    printf("    QOUT: %8.6e\n", topmodel-> Qout);
  }

  printf("\n Finalizing TOPMODEL BMI model ... \n");
  model->finalize(model);

  return 0;
}
