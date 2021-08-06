#include <stdio.h>
#include <stdlib.h>
#include "../include/bmi.h"
#include "../include/bmi_topmodel.h"
#include "../include/serialize_state.h"

/*
This main program creates two instances of a BMI-enabled version of
TOPMODEL.  It's purpose is to test a new set of BMI functions that
are intended to support model serialization for models written in C.

For Model 1, bmi.initialize() is called, and bmi.update() is called
several times to advance the model state.  Then serialize_state() is
called to retrieve the entire model state.

For Model 2, bmi.initialize() may be called, and then the function
deserialize_to_state() is called to put it in the same state as Model 1.

For both Models, bmi.update() is then called and results are compared.
*/

int main(void)
{
  const char *cfg_file = "./data/topmod.run";
  const char *ser_file = "./model_state.ser";
  int n_steps1 = 10;  // n_steps for Model1 before serializing
  int n_steps2 = 50;  // n_steps for Model2 after deserializing  
  int verbose = 1;
  int print_obj = 0;  // Set to 1 for upacking details. 

//--------------------------------------------------------------   
  if (verbose){
      puts(""); 
      puts("Allocating memory for TOPMODEL BMI model structure ...");
  }
  Bmi *model1 = (Bmi *) malloc(sizeof(Bmi));
  Bmi *model2 = (Bmi *) malloc(sizeof(Bmi));

//--------------------------------------------------------------
  if (verbose){ 
      puts("Registering TOPMODEL BMI models ...");
  }
  register_bmi_topmodel(model1);
  register_bmi_topmodel(model2);

//--------------------------------------------------------------
  if (verbose){
      puts("Initializing TOPMODEL BMI models ...");
  }
  model1->initialize(model1, cfg_file);
  model2->initialize(model2, cfg_file);    //### Needed?
  
//--------------------------------------------------------------
  if (verbose){
      puts("Updating TOPMODEL BMI model");
      printf("  n_steps1 = %i \n", n_steps1);
      puts("");
  }

  for (int i=1; i<=n_steps1; i++){
      model1->update(model1);
  }

//--------------------------------------------------------------  
  if (verbose){
      puts("Calling BMI.get_state_var_ptrs() on TOPMODEL 1 ...");
  }

  int n_state_vars;
  model1->get_state_var_count(model1, &n_state_vars);  
  void *ptr_list[ n_state_vars ];

  //---------------------------------------------
  // For testing:  All 3 print "8" on my MacPro
  //---------------------------------------------
  //printf("Size of void*    = %lu\n", sizeof(void*));
  //printf("Size of int*     = %lu\n", sizeof(int*));
  //printf("Size of double*  = %lu\n", sizeof(double*));
  //printf("\n");

  //--------------------------------------------------------------- 
  // See: https://stackoverflow.com/questions/7798383/
  //      array-of-pointers-to-multiple-types-c/7799543
  //--------------------------------------------------------------- 
  model1->get_state_var_ptrs(model1, ptr_list);

  //-------------------------------------------------------------
  // In C, we cannot infer the type or size of an array, so
  // we need BMI functions to get: name, type, size and ptr.
  // Recall that array name is a pointer to first element.
  //------------------------------------------------------------
  // Arrays are stored "flattened", so the model will take
  // care of reading values into 1D, 2D, 3D arrays.  We should
  // only need the total number of array elements.
  //------------------------------------------------------------
  if (verbose){
      printf("ptr_list[6]  = title  = %s", ptr_list[6]);  
      printf("ptr_list[8]  = dt     = %f\n", *(double *)ptr_list[8]);
      printf("ptr_list[9]  = nstep  = %d\n", *(int *)ptr_list[9]);
      printf("ptr_list[32] = Q      = %f\n", *(double *)ptr_list[32]);
      printf("ptr_list[42] = lnaotb = %f\n", *(double *)ptr_list[42]);
      printf("ptr_list[50] = cur_tstep = %d\n", *(int *)ptr_list[50]);
      printf("ptr_list[51] = sump   = %f\n", *(double *)ptr_list[51]);
      printf("ptr_list[53] = sumq   = %f\n", *(double *)ptr_list[53]);
  }
  
  //----------------------------------------------
  // NOTE! Typecast ptr first, then add offset,
  //       into array, then dereference the ptr.
  //       Careful with order of operations.
  //----------------------------------------------
  double a[3];
  a[0] = *(double *)ptr_list[56];
  a[1] = *( (double *)ptr_list[56] + 1);
  a[2] = *( (double *)ptr_list[56] + 2);
  // a[1] = *(double *)(ptr_list[56] + 1);  // This is wrong.
  // a[2] = *(double *)(ptr_list[56] + 2);
  
  if (verbose){
      printf("ptr_list[56] = dbl_arr_test = %f, %f, %f\n", a[0], a[1], a[2]);
      puts("");  // newline is added
  }

//--------------------------------------------------------------  
  if (verbose){
      puts("Calling serialize() on TOPMODEL 1 ...");
  }

  //------------------------------------------------
  // Serialize Model1 state and save to:  ser_file
  //------------------------------------------------
  serialize( model1, ser_file );

//--------------------------------------------------------------
  if (verbose){
      puts("Calling deserialize_to_state() on TOPMODEL 2 ...");
  }

  //-----------------------------------------------
  // Deserialize Model1 state saved in "ser_file"
  // and set it as the new state of Model2
  //-----------------------------------------------
  deserialize_to_state( ser_file, model2, print_obj );
  
  //--------------------------------------------------------------
  if (verbose){
      puts("Updating TOPMODEL BMI Model 2 ...");
      printf("n_steps2 = %i \n", n_steps2);
      puts("");
  }

  for (int i=1; i<=n_steps2; i++){
      model2->update(model2);
  }
 
  //--------------------------------------------------------------
  if (verbose){
      puts("Updating TOPMODEL BMI Model 1 ...");
      printf("n_steps2 = %i \n", n_steps2);
      puts("");
  }

  for (int i=1; i<=n_steps2; i++){
      model1->update(model1);
  }

  //--------------------------------------------------------------  
  if (verbose){
      puts("Calling BMI.get_state_var_ptrs() on TOPMODEL 1 ...");
  }

  model1->get_state_var_ptrs(model1, ptr_list);

  if (verbose){
      printf("ptr_list[6]  = title  = %s", ptr_list[6]);   
      printf("ptr_list[8]  = dt     = %f\n", *(double *)ptr_list[8]);
      printf("ptr_list[9]  = nstep  = %d\n", *(int *)ptr_list[9]);
      printf("ptr_list[32] = Q      = %f\n", *(double *)ptr_list[32]);
      printf("ptr_list[42] = lnaotb = %f\n", *(double *)ptr_list[42]);
      printf("ptr_list[50] = cur_tstep = %d\n", *(int *)ptr_list[50]);
      printf("ptr_list[51] = sump   = %f\n", *(double *)ptr_list[51]);
      printf("ptr_list[53] = sumq   = %f\n", *(double *)ptr_list[53]);

      //----------------------------------------------
      // NOTE! Typecast ptr first, then add offset,
      //       into array, then dereference the ptr.
      //       Careful with order of operations.
      //----------------------------------------------
      a[0] = *(double *)ptr_list[56];
      a[1] = *( (double *)ptr_list[56] + 1);
      a[2] = *( (double *)ptr_list[56] + 2);

      printf("ptr_list[56] = dbl_arr_test = %f, %f, %f\n", a[0], a[1], a[2]);
      puts("");    // newline is added
  }
 
  //--------------------------------------------------------------  
  if (verbose){
      puts("Calling BMI.get_state_var_ptrs() on TOPMODEL 2 ...");
  }

  model2->get_state_var_ptrs(model2, ptr_list);

  if (verbose){
      printf("ptr_list[6]  = title  = %s", ptr_list[6]);   
      printf("ptr_list[8]  = dt     = %f\n", *(double *)ptr_list[8]);
      printf("ptr_list[9]  = nstep  = %d\n", *(int *)ptr_list[9]);
      printf("ptr_list[32] = Q      = %f\n", *(double *)ptr_list[32]);
      printf("ptr_list[42] = lnaotb = %f\n", *(double *)ptr_list[42]);
      printf("ptr_list[50] = cur_tstep = %d\n", *(int *)ptr_list[50]);
      printf("ptr_list[51] = sump   = %f\n", *(double *)ptr_list[51]);
      printf("ptr_list[53] = sumq   = %f\n", *(double *)ptr_list[53]);

      //----------------------------------------------
      // NOTE! Typecast ptr first, then add offset,
      //       into array, then dereference the ptr.
      //       Careful with order of operations.
      //----------------------------------------------
      a[0] = *(double *)ptr_list[56];
      a[1] = *( (double *)ptr_list[56] + 1);
      a[2] = *( (double *)ptr_list[56] + 2);

      printf("ptr_list[56] = dbl_arr_test = %f, %f, %f\n", a[0], a[1], a[2]);
      puts("");    // newline is added
  }
  //--------------------------------------------------------------
  if (verbose){ 
      puts("Finalizing TOPMODEL BMI Models ...");
  }
  model1->finalize(model1);
  model2->finalize(model2);  
 
//--------------------------------------------------------------
// if (verbose){
//     printf("Freeing memory ... \n");
// }
//--------------------------------------------------------------
  if (verbose){ 
      puts("Finished with serialization test.");
      puts("");
  }
  return 0;
}
