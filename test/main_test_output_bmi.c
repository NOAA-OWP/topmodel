#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/topmodel.h" 
#include "../include/bmi.h" 
#include "../include/bmi_topmodel.h"


int
main(int argc, const char *argv[]){

    // Check for configuration file arg
    if(argc<=1){
        printf("\nmust include configuration file path...exiting\n\n");
        exit(1);
    }

    int status = BMI_SUCCESS;
    
    // Allocate model for bmi model struct
    //printf(" allocating memory for model structure...\n");
    Bmi *model = (Bmi *) malloc(sizeof(Bmi));

    // Register BMI model
    /* TODO: check if this bmi function needs to be model specific?
            or can be called 'regester_bmi()'?*/
    //printf(" registering BMI model...\n");
    register_bmi_topmodel(model);

    printf(" \ninitializing...");
    const char *cfg_file = argv[1];
    printf(" configuration found: %s\n", cfg_file);
    model->initialize(model, cfg_file);

    printf("\nBMI MODEL INFORMATION FUNCTIONS\n************************************\n");
    char name[BMI_MAX_COMPONENT_NAME];
    int count_in = 0;
    int count_out = 0;
    char **names_in = NULL;
    char **names_out = NULL;
    int i;

    // get_input_item_count()
    model->get_input_item_count(model, &count_in);
    printf(" input item count: %i\n", count_in);
    
    // get_input_var_names()
    names_in = (char**) malloc (sizeof(char *) * count_in);
    for (i=0; i<count_in; i++)
        names_in[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
    model->get_input_var_names(model, names_in);
    printf( " input variable names:\n");
    for (i=0; i<count_in; i++)
        printf("   %s\n", names_in[i]);
    
    // get_output_item_count()
    model->get_output_item_count(model, &count_out);
    printf(" output item count: %i\n", count_out);

    // get_output_var_names()
    names_out = (char**) malloc (sizeof(char *) * count_out);
    for (i=0; i<count_out; i++)
      names_out[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
    status = model->get_output_var_names(model, names_out);
    if (status == BMI_FAILURE) return BMI_FAILURE;
    printf( " output variable names:\n");
    for (i=0; i<count_out; i++)
        printf("   %s\n", names_out[i]);

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

    double *var = NULL;
    var = (double*) malloc (sizeof (double)*1);

    printf("\nBMI GETTER SETTER FUNCTIONS\n********************************\n");
    int test_nstep=100;
    double now;
    printf(" timesteps in test loop: %i\n", test_nstep);
    
    for (int n=0;n<test_nstep;n++) // shorter time loop for testing
    {

        printf(" \n setting input values...\n");
    
        // PPT get_value() old
        printf( "  %s\n", var_ppt_name);
        model->get_value(model, var_ppt_name, var);
        printf("    get value: %f\n", var[0]);
        
        // PPT set_value
        var_ppt[0] = ppt_dbl[n];
        model->set_value(model, var_ppt_name, &(var_ppt[0]));
        printf("    set value: %f\n", var_ppt[0]);
    
        // PPT get_value() new
        model->get_value(model, var_ppt_name, var);
        printf("    new value: %f\n", var[0]);


        // PET get_value() old
        printf( "  %s\n", var_pet_name);
        model->get_value(model, var_pet_name, var);
        printf("    get value: %f\n", var[0]);
        
        // PET set_value
        var_pet[0] = et_dbl[n];
        model->set_value(model, var_pet_name, &(var_pet[0]));
        printf("    set value: %f\n", var_pet[0]);

        // PET get_value() new
        model->get_value(model, var_pet_name, var);
        printf("    new value: %f\n", var[0]);

        // BMI: CONTROL FUNCTION update()
        model->update(model);
        
        // print current time
        model->get_current_time(model, &now);
        printf(" updating... current time: %f\n", now);
        for (i=0; i<count_out; i++){
            const char *var_name = names_out[i];
            // get_value() at each timestep
            model->get_value(model, var_name, var);
            printf("  %s\t %f\n", var_name, var[0]);
        }
    }
    
    // free allocation
    free(names_out); free(names_in);
    free(var); free(var_pet); free(var_ppt);

    // BMI: CONTROL FUNCTION finalize()
    printf("\n finalizing...\n\n");
    model->finalize(model);
        
    return 0;
}
