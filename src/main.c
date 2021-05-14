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
therefore, when #ifndef BMI_ACTIVE, forcings/et data not being passed to it's array
and the inital NULL value is being maintained throughout model run  
*/


int main(void)
{
  
  // allocating memory to store the entire BMI structure
  printf("\n Allocating memory to TOPMODEL BMI model structure ... \n");
  Bmi *model = (Bmi *) malloc(sizeof(Bmi));

  printf("\n Registering TOPMODEL BMI model ... \n");
  register_bmi_topmodel(model);

  const char *cfg_file = "./data/topmod.run";

  printf("\n Initializeing TOPMODEL BMI model ... \n");
  model->initialize(model, cfg_file);

  printf("\n Looping Update TOPMODEL BMI model\n");
  for (int i=1;i<=1480;i++){
    model->update(model);
  }

  //  model->update_until(model,950);

  printf("\n Finalizing TOPMODEL BMI model ... \n");
  model->finalize(model);

  return 0;
}
