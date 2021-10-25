#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/topmodel.h" 
#include "../include/bmi.h" 
#include "../include/bmi_topmodel.h"


int
main(void){

    //printf("\nBEGIN BMI UNIT TEST\n*******************\n");

    int status = BMI_SUCCESS;
    
    // Allocate model for bmi model struct
    // printf(" allocating memory for model structure...\n");
    Bmi *model = (Bmi *) malloc(sizeof(Bmi));

    // Register BMI model
    /* TODO: check if this bmi function needs to be model specific?
            or can be called 'regester_bmi()'?*/
    // printf(" registering BMI model...\n");
    register_bmi_topmodel(model);

    // Test BMI: CONTROL FUNCTION initialize()
    {
        //printf(" initializing...");
        const char *cfg_file = "./data/topmod_unit_test.run";
        //printf(" configuration found: %s\n", cfg_file);
        status = model->initialize(model, cfg_file);
        if (status == BMI_FAILURE) return BMI_FAILURE;
    }

    // Test BMI: MODEL INFORMATION FUNCTIONS
/*    printf("\nTEST BMI MODEL INFORMATION FUNCTIONS\n************************************\n");
    //char name[BMI_MAX_COMPONENT_NAME];
    int count_in = 0;
    int count_out = 0;
    char **names_in = NULL;
    char **names_out = NULL;
    int i;

    // Test get_component_name()
    {
        status = model->get_component_name(model, name);
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf(" get_component_name(): %s\n", name);
    }
    // Test get_input_item_count()
    {
        status = model->get_input_item_count(model, &count_in);
        if (status == BMI_FAILURE ) return BMI_FAILURE;
        printf(" get_input_item_count(): %i\n", count_in);
    }
    

    // Test get_input_var_names()
    { 
        names_in = (char**) malloc (sizeof(char *) * count_in);
        for (i=0; i<count_in; i++)
            names_in[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
        status = model->get_input_var_names(model, names_in);
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf( " get_input_var_names():\n");
        for (i=0; i<count_in; i++)
            printf("   %s\n", names_in[i]);
        //free(names_in);
    }
    
    // Test get_output_item_count()
    {
        status = model->get_output_item_count(model, &count_out);
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf(" get_output_item_count(): %i\n", count_out);
    }
    // Test get_output_var_names()
    {
        names_out = (char**) malloc (sizeof(char *) * count_out);
        for (i=0; i<count_out; i++)
          names_out[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
        status = model->get_output_var_names(model, names_out);
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf( " get_output_var_names():\n");
        for (i=0; i<count_out; i++)
            printf("   %s\n", names_out[i]);
        //free(names_out);
    }*/


    // Test BMI: NEW ENHANCEMENTS
    printf("\nTEST BMI NEW ENHANCEMENTS\n*************************\n");
    //int count_all = 0; //state
    int count_model_var = 0; //"all"
    int count_tmp = 0; //loop
    int count_model_var_roles = 17;  // this is hard-coded for now
    //char **names_all = NULL; //state
    char **names_model_var = NULL; //"all"
    char **names_model_var_roles = NULL;
    char **names_tmp = NULL; //loop
    char version[BMI_MAX_VERSION_NAME];
    int j, i;


        // Test get_bmi_verion()
    {
        status = model->get_bmi_version(model, version);
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf(" get_bmi_version(): %s\n", version);
    }
    // Test get_model_var_roles()
    { 
        names_model_var_roles = (char**) malloc (sizeof(char *) * count_model_var_roles);
        for (j=0; j<count_model_var_roles; j++)
            names_model_var_roles[j] = (char*) malloc (sizeof(char) * BMI_MAX_ROLE_NAME);
        status = model->get_model_var_roles(model, names_model_var_roles);
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf( " get_model_var_roles():\n");
        for (j=0; j<count_model_var_roles; j++)
            printf("  %i %s\n",j, names_model_var_roles [j]);
    }

    for (j=0; j<count_model_var_roles; j++){
        // Test get_model_var_count(some_role)
        {
            status = model->get_model_var_count(model, &count_model_var, names_model_var_roles [j]);
            if (status == BMI_FAILURE ) return BMI_FAILURE;
            printf(" get_model_var_count(%s): %i\n", names_model_var_roles [j], count_model_var);

        }
        // Test get_model_var_names(some_role)
        { 
            names_tmp = (char**) malloc (sizeof(char *) * count_model_var);
            for (i=0; i<count_model_var; i++)
                names_tmp[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
            status = model->get_model_var_names(model, names_tmp, names_model_var_roles [j]);
            if (status == BMI_FAILURE) return BMI_FAILURE;
            printf( " get_model_var_names(%s):\n", names_model_var_roles [j]);
            for (i=0; i<count_model_var; i++)
                printf("  %i %s\n", i, names_tmp[i]);
            free(names_tmp);
        }
    }
    
    free(names_model_var_roles); //don't need list of roles anymore
    
    // Check get_model_var_count("all") when null string passed
    {
        status = model->get_model_var_count(model, &count_model_var, "all");
        if (status == BMI_FAILURE ) return BMI_FAILURE;
        printf(" get_model_var_count(all): %i\n", count_model_var);
    }

/*    // Test get_state_var_count()
    {
        status = model->get_state_var_count(model, &count_all);
        if (status == BMI_FAILURE ) return BMI_FAILURE;
        printf(" state var count: %i\n", count_all);
    }
    // Test get_state_var_names()
    { 
        names_all = (char**) malloc (sizeof(char *) * count_all);
        for (i=0; i<count_all; i++)
            names_all[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
        status = model->get_state_var_names(model, names_all);
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf( " state variable names:\n");
        for (i=0; i<count_all; i++)
            printf("  %i %s\n", i, names_all[i]);
        //free(names_all);
    }*/

    // Test get_model_var_names("all")
    { 
        names_model_var = (char**) malloc (sizeof(char *) * count_model_var);
        for (i=0; i<count_model_var; i++)
            names_model_var[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
        status = model->get_model_var_names(model, names_model_var, "all");
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf( " get_model_var_names(all):\n");
        for (i=0; i<count_model_var; i++)
            printf("  %i %s\n", i, names_model_var[i]);
        //free(names_model_var);
    }
/*    // Test get_model_var_names(input)
    { 
        names_in = (char**) malloc (sizeof(char *) * count_in);
        for (i=0; i<count_in; i++)
            names_in[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
        status = model->get_model_var_names(model, names_in, "input");
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf( " get_model_var_names(input):\n");
        for (i=0; i<count_in; i++)
            printf("  %i %s\n", i, names_in[i]);
        //free(names_in);
    }
    // Test get_model_var_names(output)
    { 
        names_out = (char**) malloc (sizeof(char *) * count_out);
        for (i=0; i<count_out; i++)
            names_out[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
        status = model->get_model_var_names(model, names_out, "output");
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf( " get_model_var_names(output):\n");
        for (i=0; i<count_out; i++)
            printf("  %i %s\n", i, names_out[i]);
        //free(names_out);
    }*/



    // Test BMI: VARIABLE INFORMATION FUNCTIONS
    //printf("\nTEST BMI VARIABLE INFORMATION FUNCTIONS\n*****************************************\n");
    int grid, itemsize, nbytes;
    char type[BMI_MAX_TYPE_NAME];
    char location[BMI_MAX_LOCATION_NAME];
    char units[BMI_MAX_UNITS_NAME];
    char role[BMI_MAX_ROLE_NAME];

    // Loop through some variables and call get_var_*()
    for (i=37; i<37; i++){
        const char *var_name = names_model_var[i];
        printf( " %s\n", var_name);
        // Test get_var_grid()
        { 
            status = model->get_var_grid(model, var_name, &grid);
            if (status == BMI_FAILURE) return BMI_FAILURE;
            printf( "  get_var_grid(): %i\n", grid);
        }
        // Test get_var_itemsize()
        {
            status = model->get_var_itemsize(model, var_name, &itemsize);
            if (status == BMI_FAILURE) return BMI_FAILURE;
            printf( "  get_var_itemsize(): %i\n", itemsize);
        }
        { // Test get_var_location()
            status = model->get_var_location(model, var_name, location);
            if (status == BMI_FAILURE) return BMI_FAILURE;
            printf( "  get_var_location(): %s\n", location);
        }
        // Test get_var_units()
        { 
            status = model->get_var_units(model, var_name, units);
            if (status == BMI_FAILURE) return BMI_FAILURE;
            printf( "  get_var_units(): %s\n", units);
        }
        // Test get_var_type()
        { 
            status = model->get_var_type(model, var_name, type);
            if (status == BMI_FAILURE) return BMI_FAILURE;
            printf( "  get_var_type(): %s\n", type);
        }
        { // get_var_nbytes()
            status = model->get_var_nbytes(model, var_name, &nbytes);
            if (status == BMI_FAILURE) return BMI_FAILURE;
            printf( "  get_var_nbytes(): %i\n", nbytes);
        }
        { // Test get_var_role()
            status = model->get_var_role(model, var_name, role);
            if (status == BMI_FAILURE) return BMI_FAILURE;
            printf( "  get_var_role(): %s\n", role);
        }
    }



    // Test BMI: GET VALUE FUNCTIONS
    printf("\nTEST BMI GETTER SETTER FUNCTIONS\n********************************\n"); 
    int test_nstep=1;
    double now;
    //printf(" updating... timesteps in test loop: %i\n", test_nstep);
    
    for (int n=1;n<=test_nstep;n++) // shorter time loop for testing
    {
        // Test BMI: CONTROL FUNCTION update()
        {
            status = model->update(model);
            if (status == BMI_FAILURE) return BMI_FAILURE;
        }

        // Print current time step - function already tested
        model->get_current_time(model, &now);
        //printf("\n current time: %f\n", now);
        
        // Loop through both all variables and call get/set_value_*()
        for (i=0; i<count_model_var; i++){

            const char *var_name = names_model_var[i]; //this is all of them
    
            // The return setup will depend on role
            model->get_var_role(model, var_name, role);

            //-------------------------------------
            //             DIAGNOSTIC
            //-------------------------------------
            if (strcmp(role,"diagnostic") == 0){
                double *var = NULL;
                // Test get_value_ptr()
                {
                    status = model->get_value_ptr(model, var_name, (void**)(&var));
                    if (status == BMI_FAILURE)return BMI_FAILURE;
                    printf(" get_value_ptr(%s): %f\n",var_name,var);
                }
            }     

            //-------------------------------------
            //             INFO_STRING
            //-------------------------------------
            if (strcmp(role,"info_string") == 0){
                void *dest = NULL;
                char *this_info;
                // Test get_value_ptr()
                {
                    status = model->get_value_ptr(model, var_name, &dest);
                    this_info = (char*) dest;
                    if (status == BMI_FAILURE)return BMI_FAILURE;
                    printf(" get_value_ptr(%s): %s",var_name,this_info);
                    //free(dest);
                }
            }    

            //-------------------------------------
            //             FILE_OFFSET
            //-------------------------------------
            if (strcmp(role,"file_offset") == 0){
                void *var = NULL;
                // Test get_value_ptr()
                {
                    status = model->get_value_ptr(model, var_name, (void**)(&var));
                    if (status == BMI_FAILURE)return BMI_FAILURE;
                    printf(" get_value_ptr(%s): %f\n",var_name,var);
                    //free(var);
                }
            }
            //-------------------------------------
            //           INPUT_FROM_FILE    
            //-------------------------------------
            if (strcmp(role,"input_from_file") == 0){
                double *var = NULL;
                // Test get_value_ptr()
                {
                    status = model->get_value_ptr(model, var_name, (void**)(&var));
                    if (status == BMI_FAILURE)return BMI_FAILURE;
                    printf(" get_value_ptr(%s): %f\n",var_name,var);
                }
            }
            //-------------------------------------
            //              OPTION    
            //-------------------------------------
            if (strcmp(role,"option") == 0){
                void *var = NULL;
                int *this_option;
                // Test get_value_ptr()
                {
                    status = model->get_value_ptr(model, var_name, (&var));
                    this_option = (int*) var;
                    if (status == BMI_FAILURE)return BMI_FAILURE;
                    printf(" get_value_ptr(%s): %f\n",var_name,this_option);
                }
            }   
        }
    }


    //free(names_model_var);
    
    // Test BMI: CONTROL FUNCTION update_until()
/*    {
        int added_nstep=5;
        int total_nstep= added_nstep + test_nstep;
        printf("\n updating until... new total timesteps in test loop: %i\n", total_nstep);
        status = model->update_until(model,total_nstep);
        if (status == BMI_FAILURE) return BMI_FAILURE;
        // confirm updated current time
        model->get_current_time(model, &now);
        printf(" current time: %f\n", now);
    }*/
    // Test BMI: CONTROL FUNCTION finalize()
    {
        printf("\n finalizing...\n");
        status = model->finalize(model);
        if (status == BMI_FAILURE) return BMI_FAILURE;
        printf("\n******************\nEND BMI UNIT TEST\n\n");
    }
    return 0;
}
