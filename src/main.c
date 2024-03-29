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
#if TOPMODEL_DEBUG > 1  
  printf("\n Allocating memory to TOPMODEL BMI model structure ... \n");
#endif
  Bmi *model = (Bmi *) malloc(sizeof(Bmi));

#if TOPMODEL_DEBUG > 1  
  printf("\n Registering TOPMODEL BMI model ... \n");
#endif
  register_bmi_topmodel(model);

  const char *cfg_file = "./data/topmod.run";

#if TOPMODEL_DEBUG > 1  
  printf("\n Initializeing TOPMODEL BMI model ... \n");
#endif
  model->initialize(model, cfg_file);

#if TOPMODEL_DEBUG > 1  
  printf("\n Looping Update TOPMODEL BMI model\n");
#endif
  
  // Get number of timesteps
  topmodel_model *topmodel;
  topmodel = (topmodel_model *) model->data;
  int n;
  
  if (topmodel->stand_alone == TRUE){
    // Gather number of steps from input file
    // when in standalone mode.
    n = topmodel->nstep;
    //NJF in standalone mode, one MUST allocate Q for all time steps
    //This should probably re-considered and additional validation done
    //that the algorithms work as intended
    //especially with the addition of `shift_Q` in the topmodel function
    //but for now, this prevents a fault
    if(topmodel->Q != NULL) free(topmodel->Q);
    d_alloc(&topmodel->Q, n);
  }
  else{
    // Otherwise define loop here
    // Note: this is a pseudo-framework
    n = 720;
  }

  for (int i=1;i<=n;i++){
    model->update(model);
  }

  //  model->update_until(model,950);

#if TOPMODEL_DEBUG > 1  
  printf("\n Finalizing TOPMODEL BMI model ... \n");
#endif
  model->finalize(model);

  free(model);
  return 0;
}
