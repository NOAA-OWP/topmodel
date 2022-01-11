#include "../include/topmodel.h" 
#include "../include/bmi.h" 
#include "../include/bmi_topmodel.h"

/* BMI Adaption: Max i/o file name length changed from 30 to 256 */
#define MAX_FILENAME_LENGTH 256

//----------------------------------------------
// Put variable info into a struct to simplify
// BMI implementation and avoid errors.
//----------------------------------------------
// Should we add "/0" after each string here?
// Everything works without it.
//---------------------------------------------- 
// {idx, name, type, size, role, units, grid, location}
//----------------------------------------------
// role is new to bmi enhancement
// valid role options via 'model_var_roles' 
//    "array_length",
//    "constant",
//    "diagnostic",
//    "directory",
//    "filename",
//    "file_offset",
//    "info_string",
//    "input_from_bmi",
//    "input_from_file",
//    "not_set",
//    "option",
//    "output_to_bmi",
//    "output_to_file",
//    "parameter_fixed",
//    "parameter_adjustable",
//    "state",
//    "time_info"
//----------------------------------------------
// Note only 'input' and 'output' need to follow
// CSDMS standard names. See
// https://csdms.colorado.edu/wiki/CSDMS_Standard_Names
//----------------------------------------------
Variable var_info[] = {
    //-------------------------------------
    // File pointers.  For reference only
    //-------------------------------------
    { 0,  "control_fptr", "FILE", 1, "file_offset", "none", "node", 0 },
    { 1,  "input_fptr",   "FILE", 1, "file_offset", "none", "node", 0 },
    { 2,  "subcat_fptr",  "FILE", 1, "file_offset", "none", "node", 0 },
    { 3,  "params_fptr",  "FILE", 1, "file_offset", "none", "node", 0 },
    { 4,  "output_fptr",  "FILE", 1, "file_offset", "none", "node", 0 },
    { 5,  "out_hyd_fptr", "FILE", 1, "file_offset", "none", "node", 0 },
    //----------------------------------------------
    // String vars.  Will replace 1 w/ title_size.
    //----------------------------------------------
    { 6,  "title",  "string", 1, "info_string", "none", "node", 0 },
    { 7,  "subcat", "string", 1, "info_string", "none", "node", 0 },   
    //-----------------------
    // Variable definitions
    //-----------------------
    { 8,  "dt",                 "double", 1, "time_info", "h-1", "node", 0 },    //inputs.dat
    { 9,  "nstep",              "int",    1, "time_info", "none", "node", 0 },   //inputs.dat
    { 10, "yes_print_output",   "int",    1, "option", "none", "node", 0 }, //subcat.dat
    { 11, "imap",               "int",    1, "option", "none", "node", 0 }, //subcat.dat
    { 12, "num_channels",       "int",    1, "parameter_fixed", "none", "node", 0 },  //subcat.dat
    { 13, "num_topodex_values", "int",    1, "parameter_fixed", "none", "node", 0 },  //subcat.dat
    { 14, "infex",              "int",    1, "option", "none", "node", 0 }, //infiltration           
    //-------------------------------------
    // Model parameters and input scalars
    //-------------------------------------
    { 15,  "szm",        "double", 1, "parameter_adjustable", "m", "node", 0 }, 
    { 16,  "t0",         "double", 1, "parameter_fixed", "none", "node", 0 }, //areal average of ln(a/tanB)
    { 17,  "td",         "double", 1, "parameter_adjustable", "h-1", "node", 0 }, //unsaturated zome time delay per unit storage deficit
    { 18,  "srmax",      "double", 1, "parameter_adjustable", "m", "node", 0 },   //maximum root zone storage deficit
    { 19,  "Q0",         "double", 1, "state", "m h-1", "node", 0 }, //initial subsurface flow per unit area //JG TODO: check this one
    { 20,  "sr0",        "double", 1, "state", "m", "node", 0 }, //initial root zone storage deficit
    { 21,  "xk0",        "double", 1, "parameter_adjustable", "m h-1", "node", 0 }, //surface soil hydraulic conductivity
    { 22,  "hf",         "double", 1, "parameter_adjustable", "m", "node", 0 }, //wetting front suction for G&A soln.
    { 23,  "dth",        "double", 1, "parameter_adjustable", "none", "node", 0 }, //water content change across the wetting front
    { 24,  "area",       "double", 1, "parameter_fixed", "none", "node", 0 }, //subcat.dat
    { 25,  "num_delay",  "int",    1, "parameter_fixed", "none", "node", 0 }, //number of time steps lag (delay) in channel within catchment to outlet 
    { 26,  "num_time_delay_histo_ords",  "int", 1, "parameter_fixed", "none", "node", 0}, //number of time delay histogram ordinates */
    { 27,  "szq",              "double", 1, "parameter_fixed", "none", "node", 0 },
    { 28,  "tl",               "double", 1, "parameter_fixed", "none", "node", 0 },
    { 29,  "max_contrib_area", "double", 1, "parameter_fixed", "none", "node", 0 }, //could be option
    { 30,  "land_surface_water__water_balance_volume", "double", 1, "output_to_file", "m", "node", 0 }, //bal //residual of the water balance
    { 31,  "soil_water__domain_volume_deficit",        "double", 1, "output_to_bmi", "m", "node", 0 }, //sbar //catchment average soil moisture deficit
    //------------------------------------------------
    // Pointers to dynamically dimensioned 1D arrays
    // Will replace size of 1 with size in comment
    // once those vars are defined.
    //------------------------------------------------
    // A trailing asterisk indicates that the var
    // is actually a pointer to the given type.
    //------------------------------------------------ 
    { 32,  "land_surface_water__runoff_mass_flux", "double*", 1, "output_to_bmi", "m h-1", "node", 0 }, // n_steps //Q //simulated discharge
    { 33,  "Qobs",                     "double*", 1, "input_from_file", "m h-1", "node", 0 },  // n_steps
    { 34,  "atmosphere_water__liquid_equivalent_precipitation_rate", "double*", 1, "input_from_bmi", "m h-1", "node", 0 },  // n_steps //rain //inputs.dat
    { 35,  "water_potential_evaporation_flux",                       "double*", 1, "input_from_bmi", "m h-1", "node", 0 },  // n_steps //pe //inputs.dat
    { 36,  "contrib_area",             "double*", 1, "state", "none", "node", 0 },    // n_steps
    { 37,  "stor_unsat_zone",          "double*", 1, "state", "m", "node", 0 },  // max_atb_incs
    { 38,  "deficit_root_zone",        "double*", 1, "state", "m", "node", 0 },  // max_atb_incs
    { 39,  "deficit_local",            "double*", 1, "state", "m", "node", 0 },  // max_atb_incs
    { 40,  "time_delay_histogram",     "double*", 1, "parameter_fixed", "none", "node", 0 },  // max_td_ords
    { 41,  "dist_area_lnaotb",         "double*", 1, "parameter_fixed", "none", "node", 0 },  // max_n_incs
    { 42,  "lnaotb",                   "double*", 1, "parameter_fixed", "none", "node", 0 },  // max_n_incs
    { 43,  "cum_dist_area_with_dist",  "double*", 1, "parameter_fixed", "none", "node", 0 },  // max_n_subcats
    { 44,  "dist_from_outlet",         "double*", 1, "parameter_fixed", "m", "node", 0 },      // max_n_subcats   
    //---------------------- 
    // Other internal vars
    //----------------------
    { 45,  "num_sub_catchments",       "int", 1, "array_length", "none", "node", 0 }, //subcat.dat
    { 46,  "max_atb_increments",       "int", 1, "array_length", "none", "node", 0 },
    { 47,  "max_num_subcatchments",    "int", 1, "array_length", "none", "node", 0 },
    { 48,  "max_time_delay_ordinates", "int", 1, "array_length", "none", "node", 0 },
    { 49,  "Qout",                     "double", 1, "state", "m h-1", "node", 0 }, // Output var  //runoff at timestep further converted using width func
    //---------------------- 
    // BMI vars
    //----------------------    
    { 50,  "current_time_step",        "int", 1, "time_info", "none", "node", 0 },    // BMI var
    //-----------------
    // State var sums
    //-----------------
    { 51,  "land_surface_water__domain_time_integral_of_precipitation_volume_flux", "double", 1, "diagnostic", "m", "node", 0 },
    { 52,  "land_surface_water__domain_time_integral_of_evaporation_volume_flux",   "double", 1, "diagnostic", "m", "node", 0 },
    { 53,  "land_surface_water__domain_time_integral_of_runoff_volume_flux",        "double", 1, "diagnostic", "m", "node", 0 },
    { 54,  "soil_water__domain_root-zone_volume_deficit",                           "double", 1, "diagnostic", "m", "node", 0 },
    { 55,  "soil_water__domain_unsaturated-zone_volume",                            "double", 1, "diagnostic", "m", "node", 0 },
    //----------------------    
    // External/forcing vars
    //----------------------
    { 56, "soil_water_root-zone_unsat-zone_top__recharge_volume_flux",             "double", 1, "output_to_file", "m", "node", 0 },
    { 57, "land_surface_water__baseflow_volume_flux",                              "double", 1, "output_to_file", "m", "node", 0 },
    { 58, "land_surface_water__domain_time_integral_of_overland_flow_volume_flux", "double", 1, "output_to_file", "m h-1", "node", 0 },
    { 59, "atmosphere_water__domain_time_integral_of_rainfall_volume_flux",        "double", 1, "output_to_file", "m h-1", "node", 0 },
    { 60, "land_surface_water__potential_evaporation_volume_flux",                 "double", 1, "output_to_file", "m h-1", "node", 0 },
    { 61, "stand_alone",                                                           "int",    1, "option", "none", "node", 0 }
    // { 62, "obs_values",      "double", 1 },    
    // { 63, "double_arr_test", "double", 3 }             
};

static const char *model_var_roles[] = {
    "array_length",
    "constant",
    "diagnostic",
    "directory",
    "filename",
    "file_offset",
    "info_string",
    "input_from_bmi",
    "input_from_file",
    "not_set",
    "option",
    "output_to_bmi",
    "output_to_file",
    "parameter_fixed",
    "parameter_adjustable",
    "state",
    "time_info"
};

// These replace hard-coded #DEFINE so you don't have to keep updating, yey
int VAR_NAME_COUNT = sizeof(var_info)/sizeof(var_info[0]);
int VAR_ROLE_COUNT = sizeof(model_var_roles)/sizeof(model_var_roles[0]);

int read_init_config(const char* config_file, topmodel_model* model) {
    
    // Open the primary config file
    /* BMI Adaption: No longer needs to be named "topmod.run" */
    if((model->control_fptr=fopen(config_file,"r"))==NULL){
        printf("Can't open control file named %s\n",config_file);      
        exit(-9);
    }

    /* BMI Adaption: Include stand_alone (bool) in config

    Structure of config_file as follows: 
    stand_alone
    title
    path/to/inputs.dat
    path/to/subcat.dat
    path/to/params.dat
    path/to/topmod.out
    path/to/hyd.out */

    //Read the stand_alone T/F
    //note: newline is needed here!
    fscanf(model->control_fptr,"%d\n",&model->stand_alone);

    //Read the title line, up to 255 characters, of the the file
    fgets(model->title,256,model->control_fptr);
    
    //Read a string, breaks on whitespace (or newline)
    //These must be done IN ORDER
    char input_fname[MAX_FILENAME_LENGTH];
    //It might be worth always scanning this line, but only opening the file if not STAND_ALONE
    fscanf(model->control_fptr,"%s",input_fname);
    
    //If stand_alone TRUE, read inputs from input file
    if (model->stand_alone == TRUE){
        if((model->input_fptr=fopen(input_fname,"r"))==NULL){
            printf("Can't open input file named %s\n",input_fname);
            exit(-9);
        }
    };

    char subcat_fname[MAX_FILENAME_LENGTH],params_fname[MAX_FILENAME_LENGTH];
    fscanf(model->control_fptr,"%s",subcat_fname);
    fscanf(model->control_fptr,"%s",params_fname);

    char output_fname[MAX_FILENAME_LENGTH],out_hyd_fname[MAX_FILENAME_LENGTH];
    fscanf(model->control_fptr,"%s",output_fname);
    fscanf(model->control_fptr,"%s",out_hyd_fname);

    //Attempt to read the parsed input file names, bail if they cannot be read/created
    if((model->subcat_fptr=fopen(subcat_fname,"r"))==NULL){       
        printf("Can't open subcat file named %s\n",subcat_fname);
        exit(-9);
    }

    if((model->params_fptr=fopen(params_fname,"r"))==NULL){
        printf("Can't open params file named %s\n",params_fname);   
        exit(-9);
    }
    
    /* READ IN SUBCATCHMENT TOPOGRAPHIC DATA */
    // This is needed here to gather yes_print_output for possible outfile read-in
    // TODO: JG thought - 
    //      If framework will never want these out files, 
    //      just use model->stand_alone as control switch
    //      move line to init_config() with others
    fscanf(model->subcat_fptr,"%d %d %d",&model->num_sub_catchments,&model->imap,&model->yes_print_output);

    // Attempt to read the output file names only if printing to file
    if(model->yes_print_output == TRUE){
        if((model->output_fptr=fopen(output_fname,"w"))==NULL){           
            printf("Can't open output file named %s\n",output_fname);
            exit(-9);
        }

        if((model->out_hyd_fptr=fopen(out_hyd_fname,"w"))==NULL){          
            printf("Can't open output file named %s\n",out_hyd_fname);
            exit(-9);
        }

        fprintf(model->output_fptr,"%s\n",model->title);

    }    
 
#if TOPMODEL_DEBUG >= 1    
    printf("TOPMODEL Version: TMOD95.02\n");
    printf("This run: %s\n",model->title);
#endif

    fclose(model->control_fptr);
    // Note all individual input files closed in init_config(),
    // which calls this function read_init_config()
    // Output files (if opened) closed in finalize()

    return BMI_SUCCESS;

}

int init_config(const char* config_file, topmodel_model* model)
{
    read_init_config(config_file,model);
    
    if (model->stand_alone == TRUE){
        /* READ IN nstep, DT and RAINFALL, PE, QOBS INPUTS */
        inputs(model->input_fptr, &model->nstep, &model->dt, &model->rain, &model->pe, 
            &model->Qobs, &model->Q, &model->contrib_area);
        fclose(model->input_fptr);
    }
    else {
        
        /* Set nstep and dt*/
        model->nstep = 1;
        model->dt = 1.0;

        /* allocate memory for "arrays" */
        d_alloc(&model->rain,model->nstep);
        d_alloc(&model->pe,model->nstep);
        d_alloc(&model->Qobs,model->nstep);   //TODO: Consider removing this all together when framework
        d_alloc(&model->Q,model->nstep);
        d_alloc(&model->contrib_area,model->nstep);

        (model->rain)[1]=0.0;
        (model->pe)[1]=0.0;
        (model->Qobs)[1]=0.0;
        (model->Q)[1]=0.0;
        (model->contrib_area)[1]=0.0;
    }

    // Set up maxes for subcat and params read-in functions
    model-> max_atb_increments=30;
    model-> max_num_subcatchments=10;
    model-> max_time_delay_ordinates=20;

    tread(model->subcat_fptr,model->output_fptr,model->subcat,&model->num_topodex_values,&model->num_channels,
        &model->area,&model->dist_area_lnaotb,&model->lnaotb,model->yes_print_output,
        &model->cum_dist_area_with_dist,&model->tl,&model->dist_from_outlet,
        model->max_num_subcatchments,model->max_atb_increments);
    fclose(model->subcat_fptr);

    init(model->params_fptr,model->output_fptr,model->subcat,model->num_channels,model->num_topodex_values,
        model->yes_print_output,model->area,&model->time_delay_histogram,model->cum_dist_area_with_dist,
        model->dt,&model->szm,&model->t0,model->tl,model->dist_from_outlet,&model->td, &model->srmax,&model->Q0,&model->sr0,&model->infex,&model->xk0,&model->hf,
        &model->dth,&model->stor_unsat_zone,&model->deficit_local,&model->deficit_root_zone,
        &model->szq,model->Q,&model->sbar,model->max_atb_increments,model->max_time_delay_ordinates,
        &model->bal,&model->num_time_delay_histo_ords,&model->num_delay);
    fclose(model->params_fptr);


    return BMI_SUCCESS;
}

// ***********************************************************
// ******************* BMI: TIME FUNCTIONS *******************
// ***********************************************************

static int Get_start_time (Bmi *self, double * time)
{
    *time = 0.0;
    return BMI_SUCCESS;
}

static int Get_end_time (Bmi *self, double * time)
{

    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;
    Get_start_time(self, time);
    
    // Standalone case gathers end_time via nstep dt
    if (topmodel->stand_alone == TRUE){
        *time += topmodel->nstep * topmodel->dt;
        return BMI_SUCCESS;
    
    // Otherwise, set to FLT_MAX macro via float.h
    // See https://bmi.readthedocs.io/en/latest/#get-end-time
    }
    else {
        *time += FLT_MAX;
        return BMI_SUCCESS;
    }
}

static int Get_time_step (Bmi *self, double * dt)
{
    *dt = ((topmodel_model *) self->data)->dt;
    return BMI_SUCCESS;
}

static int Get_time_units (Bmi *self, char * units)
{
    strncpy (units, "h", BMI_MAX_UNITS_NAME);
    return BMI_SUCCESS;
}

static int Get_current_time (Bmi *self, double * time)
{
    Get_start_time(self, time);
#if TOPMODEL_DEBUG > 1
    printf("Current model time step: '%d'\n", ((topmodel_model *) self->data)->current_time_step);
#endif
    *time += (((topmodel_model *) self->data)->current_time_step * 
              ((topmodel_model *) self->data)->dt);
    return BMI_SUCCESS;
}


// ***********************************************************
// *************** BMI: MODEL CONTROL FUNCTIONS **************
// ***********************************************************

static int Initialize (Bmi *self, const char *cfg_file)
{
    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;

    // Read and setup data from file
    init_config(cfg_file, topmodel);

    // Initialize model varables which are cumulatively defined
    topmodel->current_time_step=0;
    topmodel->sump = 0.0;
    topmodel->sumae = 0.0;
    topmodel->sumq = 0.0;

    return BMI_SUCCESS;
}

// This is not needed. Update_until does it already.
// static int Update_frac (void * self, double f)
// { /* Implement this: Update for a fraction of a time step */
//     return BMI_FAILURE;
// }

static int Update (Bmi *self)
{
    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;

    double current_time, end_time;
    self->get_current_time(self, &current_time);
    self->get_end_time(self, &end_time);
    if (current_time >= end_time) {
        return BMI_FAILURE;
    };

    topmodel->current_time_step += topmodel->dt;   

    topmod(topmodel->output_fptr,topmodel->nstep, topmodel->num_topodex_values,
        topmodel->yes_print_output,topmodel->infex, topmodel->dt, topmodel->szm,
        topmodel->stor_unsat_zone,topmodel->deficit_root_zone,
        topmodel->deficit_local, topmodel->pe, topmodel->rain,topmodel->xk0,topmodel->hf, 
        topmodel->dist_area_lnaotb, topmodel->tl, topmodel->lnaotb, topmodel->td,
        topmodel->srmax, topmodel->contrib_area, topmodel->szq, &topmodel->Qout, 
        topmodel->num_time_delay_histo_ords,topmodel->Q,
        topmodel->time_delay_histogram,topmodel->subcat,&topmodel->bal,
        &topmodel->sbar,topmodel->num_delay,topmodel->current_time_step, topmodel->stand_alone,
        &topmodel->sump,&topmodel->sumae,&topmodel->sumq,&topmodel->sumrz,&topmodel->sumuz,
        &topmodel->quz, &topmodel->qb, &topmodel->qof, &topmodel->p, &topmodel->ep );

    //--------------------------------------------------
    // This should be moved into the Finalize() method
    //--------------------------------------------------
    // results() 
    // 1. generates hydrograph out file (hyd.out)
    // 2. computes objective function stats
    //        - print to console
    //        - print to main out file (topmod.out)
    // Logic for each is handled indiv w.i. funct,
    // but wouldn't hurt to check conditions here as framework
    // will likely not even need to jump into results()
    if (topmodel->stand_alone == TRUE){
    results(topmodel->output_fptr, topmodel->out_hyd_fptr, topmodel->nstep, 
        topmodel->Qobs, topmodel->Q, 
        topmodel->current_time_step, topmodel->yes_print_output);
    }

    return BMI_SUCCESS;
}

static int Update_until (Bmi *self, double t)
{
    double dt;
    double now;

    if(self->get_time_step (self, &dt) == BMI_FAILURE)
        return BMI_FAILURE;

    if(self->get_current_time(self, &now) == BMI_FAILURE)
        return BMI_FAILURE;

    {
    
    int n;
    double frac;
    const double n_steps = (t - now) / dt;
    for (n=0; n<(int)n_steps; n++) {
        Update(self);
    }
    frac = n_steps - (int)n_steps;
    if (frac > 0){
        ((topmodel_model *)self->data)->dt = frac * dt;
        Update (self);
        ((topmodel_model *)self->data)->dt = dt;
    }
    
    }

    return BMI_SUCCESS;
}

static int Finalize (Bmi *self)
{
  if (self){
    topmodel_model* model = (topmodel_model *)(self->data);

    //-----------------------------------------------------------
    // When running in stand-alone mode, the original "results"
    // method should be called here in the Finalize() method,
    // not in Update_until(). It could also be called when in
    // framework-controlled mode.
    //-----------------------------------------------------------
    //if (model->yes_print_output == TRUE || TOPMODEL_DEBUG >= 1){
    //results(model->output_fptr,model->out_hyd_fptr,model->nstep, 
    //    model->Qobs, model->Q, 
    //    model->current_time_step, model->yes_print_output);
    
    if (model->yes_print_output == TRUE || TOPMODEL_DEBUG >= 1){        
        
        water_balance(model->output_fptr, model->yes_print_output,
            model->subcat,&model->bal, &model->sbar, &model->sump, 
            &model->sumae, &model->sumq, &model->sumrz, &model->sumuz);
    }

    if( model->Q != NULL )
        free(model->Q);
    if( model->Qobs != NULL )
        free(model->Qobs);
    if( model->rain != NULL )   // NOTE: invalid pointer when standalone TRUE
        free(model->rain);
    if( model->pe != NULL )
        free(model->pe);
    if( model->contrib_area != NULL )
        free(model->contrib_area);
    if( model->stor_unsat_zone != NULL )
        free(model->stor_unsat_zone);
    if( model->deficit_root_zone != NULL )
        free(model->deficit_root_zone);
    if( model->deficit_local != NULL )
        free(model->deficit_local);
    if( model->time_delay_histogram != NULL )
        free(model->time_delay_histogram);
    if( model->dist_area_lnaotb != NULL )
        free(model->dist_area_lnaotb);
    if( model->lnaotb != NULL )
        free(model->lnaotb);
    if( model->cum_dist_area_with_dist != NULL )
        free(model->cum_dist_area_with_dist);
    if( model->dist_from_outlet != NULL)
        free(model->dist_from_outlet);

    // Close output files only if opened in first place
    if(model->yes_print_output == TRUE){
        fclose(model->output_fptr);
        fclose(model->out_hyd_fptr);
    }
    
    free(self->data);
  }
    return BMI_SUCCESS;
}


// ***********************************************************
// *********** BMI: VARIABLE INFORMATION FUNCTIONS ***********
// ***********************************************************

static int Get_var_type (Bmi *self, const char *name, char * type)
{
    // JG 10.07.21: DONE

/*    // Check to see if in output array first
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, output_var_names[i]) == 0) {
            strncpy(type, output_var_types[i], BMI_MAX_TYPE_NAME);
            return BMI_SUCCESS;
        }
    }
    // Then check to see if in input array
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            strncpy(type, input_var_types[i], BMI_MAX_TYPE_NAME);
            return BMI_SUCCESS;
        }
    }*/

    // NEW BMI EXTENSION
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp(name, var_info[i].name) == 0) {
            strncpy(type, var_info[i].type, BMI_MAX_TYPE_NAME);
            return BMI_SUCCESS;
        }    
    }
    
    // If we get here, it means the variable name wasn't recognized
    type[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_grid(Bmi *self, const char *name, int *grid)
{
    // JG 10.07.21: DONE

/*    // Check to see if in output array first
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, output_var_names[i]) == 0) {
            *grid = output_var_grids[i];
            return BMI_SUCCESS;
        }
    }
    // Then check to see if in input array
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            *grid = input_var_grids[i];
            return BMI_SUCCESS;
        }
    }*/
    
    // NEW BMI EXTENSION
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp(name, var_info[i].name) == 0) {
            *grid = var_info[i].grid;
            return BMI_SUCCESS;
        }    
    }

    // If we get here, it means the variable name wasn't recognized
    grid[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_itemsize (Bmi *self, const char *name, int * size)
{
    char type[BMI_MAX_TYPE_NAME];
    int type_result = Get_var_type(self, name, type);
    if (type_result != BMI_SUCCESS) {
        return BMI_FAILURE;
    }

    // NEW BMI EXTENSION
    if ((strcmp (type, "double") == 0) | (strcmp (type, "double*") == 0)) {
        *size = sizeof(double);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "float") == 0) {
        *size = sizeof(float);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "int") == 0) {
        *size = sizeof(int);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "short") == 0) {
        *size = sizeof(short);
        return BMI_SUCCESS;
    }
    else if (strcmp (type, "long") == 0) {
        *size = sizeof(long);
        return BMI_SUCCESS;
    }
    // NEW BMI EXTENSION
    // Note: This returns sizeof 1; Get_var_nbytes() considers length/size
    // JG TODO: Confirm OK?
    else if ((strcmp (type, "FILE") == 0) | (strcmp (type, "string") == 0)) {
        *size = sizeof(char);
        return BMI_SUCCESS;
    }
    else {
        *size = 0;
        return BMI_FAILURE;
    }
 
}

static int Get_var_location (Bmi *self, const char *name, char * location)
{
    // JG 10.07.21: DONE

/*    // Check to see if in output array first
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, output_var_names[i]) == 0) {
            strncpy(location, output_var_locations[i], BMI_MAX_LOCATION_NAME);
            return BMI_SUCCESS;
        }
    }
    // Then check to see if in input array
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            strncpy(location, input_var_locations[i], BMI_MAX_LOCATION_NAME);
            return BMI_SUCCESS;
        }
    }*/

    // NEW BMI EXTENSION
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp(name, var_info[i].name) == 0) {
            strncpy(location, var_info[i].location, BMI_MAX_LOCATION_NAME);
            return BMI_SUCCESS;
        }    
    }
    
    // If we get here, it means the variable name wasn't recognized
    location[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_units (Bmi *self, const char *name, char * units)
{
    // JG 10.07.21: DONE

/*    // Check to see if in output array first
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, output_var_names[i]) == 0) {
            strncpy(units, output_var_units[i], BMI_MAX_UNITS_NAME);
            return BMI_SUCCESS;
        }
    }
    // Then check to see if in input array
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            strncpy(units, input_var_units[i], BMI_MAX_UNITS_NAME);
            return BMI_SUCCESS;
        }
    }*/

    // NEW BMI EXTENSION
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp(name, var_info[i].name) == 0) {
            strncpy(units, var_info[i].units, BMI_MAX_UNITS_NAME);
            return BMI_SUCCESS;
        }    
    }

    // If we get here, it means the variable name wasn't recognized
    units[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_nbytes (Bmi *self, const char *name, int * nbytes)
{
    // JG 10.07.21: Not 100% on this one, why condition < 1?

    int each_item_size;
    int item_size_result = Get_var_itemsize(self, name, &each_item_size);
    if (item_size_result != BMI_SUCCESS) {
        return BMI_FAILURE;
    }
/*    int item_count = -1;
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        if (strcmp(name, input_var_names[i]) == 0) {
            item_count = input_var_item_count[i];
            break;
        }
    }
    if (item_count < 1) {
        for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
            if (strcmp(name, output_var_names[i]) == 0) {
                item_count = output_var_item_count[i];
                break;
            }
        }
    }
    if (item_count < 1)
        item_count = ((topmodel_model *) self->data)->nstep;*/

    // NEW BMI EXTENSION
    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;

    //-----------------------------------------------------
    // NOTE:  TOPMODEL uses d_alloc() to allocate memory
    //        for arrays, but adds 1 to the array size
    //        and then loop counts start at 1 not 0.
    //        Also, d_alloc() is often called with a +1.
    //        So we need to add 1 to all of these.
    //-----------------------------------------------------
    unsigned int title_size = 257;   // see topmodel.h; char array
    unsigned int n_steps       = topmodel->nstep+1;
    unsigned int max_n_subcats = topmodel->max_num_subcatchments+1;
    unsigned int max_n_incs    = topmodel->max_atb_increments+1;
    unsigned int max_atb_incs  = topmodel->max_atb_increments+1;
    unsigned int max_td_ords   = topmodel->max_time_delay_ordinates+1;
  

    // JG TODO: change these to strcmp() vs index?

    //-------------------------------------------------
    // Overwrite the sizes that are not 1 (now known)
    //-------------------------------------------------
    var_info[6].size  = title_size;     // title
    var_info[7].size  = title_size;     // subcat
    //---------------------------------------------    
    var_info[32].size = n_steps;        // Q
    var_info[33].size = n_steps;        // Qobs
    var_info[34].size = n_steps;        // rain
    var_info[35].size = n_steps;        // pe
    var_info[36].size = n_steps;        // contrib_area
    var_info[37].size = max_atb_incs;   // stor_unsat_zone
    var_info[38].size = max_atb_incs;   // deficit_root_zone
    var_info[39].size = max_atb_incs;   // deficit_local
    var_info[40].size = max_td_ords;    // time_delay_histogram
    var_info[41].size = max_n_incs;     // dist_area_lnaotb
    var_info[42].size = max_n_incs;     // lnaotb
    var_info[43].size = max_n_subcats;  // cum_dist_area_with_dist
    var_info[44].size = max_n_subcats;  // dist_from_outlet

    // No we know #ofElems, so xBy each item_size    
    int item_count;
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp(name, var_info[i].name) == 0) {
            item_count = var_info[i].size;
            *nbytes = each_item_size * item_count;
            return BMI_SUCCESS;
        }    
    }

    return BMI_FAILURE;
}

/* OWP Custom BMI Enhancements */
static int Get_var_length (Bmi *self, const char *name, int * elements)
{
    // Returns size of varibleable array
    // consider get_var_size?

    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;

    //-----------------------------------------------------
    // NOTE:  TOPMODEL uses d_alloc() to allocate memory
    //        for arrays, but adds 1 to the array size
    //        and then loop counts start at 1 not 0.
    //        Also, d_alloc() is often called with a +1.
    //        So we need to add 1 to all of these.
    //-----------------------------------------------------
    unsigned int title_size = 257;   // see topmodel.h; char array
    unsigned int n_steps       = topmodel->nstep+1;
    unsigned int max_n_subcats = topmodel->max_num_subcatchments+1;
    unsigned int max_n_incs    = topmodel->max_atb_increments+1;
    unsigned int max_atb_incs  = topmodel->max_atb_increments+1;
    unsigned int max_td_ords   = topmodel->max_time_delay_ordinates+1;
  

    // JG TODO: change these to strcmp() vs index?

    //-------------------------------------------------
    // Overwrite the sizes that are not 1 (now known)
    //-------------------------------------------------
    var_info[6].size  = title_size;     // title
    var_info[7].size  = title_size;     // subcat
    //---------------------------------------------    
    var_info[32].size = n_steps;        // Q
    var_info[33].size = n_steps;        // Qobs
    var_info[34].size = n_steps;        // rain
    var_info[35].size = n_steps;        // pe
    var_info[36].size = n_steps;        // contrib_area
    var_info[37].size = max_atb_incs;   // stor_unsat_zone
    var_info[38].size = max_atb_incs;   // deficit_root_zone
    var_info[39].size = max_atb_incs;   // deficit_local
    var_info[40].size = max_td_ords;    // time_delay_histogram
    var_info[41].size = max_n_incs;     // dist_area_lnaotb
    var_info[42].size = max_n_incs;     // lnaotb
    var_info[43].size = max_n_subcats;  // cum_dist_area_with_dist
    var_info[44].size = max_n_subcats;  // dist_from_outlet

    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp(name, var_info[i].name) == 0) {
            *elements = var_info[i].size;
            return BMI_SUCCESS;
        }
    }
  
    return BMI_FAILURE;
}

/* OWP Custom BMI Enhancements */
static int Get_var_role (Bmi *self, const char *name, char * role)
{
    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;

    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp(name, var_info[i].name) == 0) {
            //----------------------------------------------------
            // Override some roles based on config file settings
            //----------------------------------------------------
            if (topmodel->stand_alone == TRUE) {              
                // input_from_bmi --> input_from_file 
                if (strcmp(var_info[i].role, "input_from_bmi") == 0){
                    //role = "input_from_file";  //this doesn't work
                    strncpy(role, "input_from_file", BMI_MAX_ROLE_NAME);
                    return BMI_SUCCESS; 
                }
                // output_to_bmi --> output_to_file 
                if (strcmp(var_info[i].role, "output_to_bmi") == 0){
                    //role = "output_to_file";
                    strncpy(role, "output_to_file", BMI_MAX_ROLE_NAME);
                    return BMI_SUCCESS; 
                }
            }               

            strncpy(role, var_info[i].role, BMI_MAX_ROLE_NAME);
            return BMI_SUCCESS;  
        }  

    }
    
    //--------------------------
    // No match found for name
    //--------------------------
    printf("ERROR in get_var_role():\n");
    printf("  No match for: %s\n\n", name); 
    //role = "not_set";
    strncpy(role, "not_set", BMI_MAX_ROLE_NAME);
    return BMI_FAILURE;
}

/* OWP Custom BMI Enhancements */
static int Get_var_index (Bmi *self, const char *name, int *index)
{
    //-------------------------------------------------
    // Note: This pulls information from the var_info
    // structure defined at the top, which helps to 
    // prevent implementation errors.   
    //-------------------------------------------------
    if (!self){
        return BMI_FAILURE;   
    }
    
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp( var_info[i].name, name ) == 0){
            *index = var_info[i].index;
            return BMI_SUCCESS;
        }
    }

    //--------------------------
    // No match found for name
    //--------------------------
    printf("ERROR in get_var_index():\n");
    printf("  No match for: %s\n\n", name);
    *index = -1;
    return BMI_FAILURE;
}

// ***********************************************************
// ********* BMI: VARIABLE GETTER & SETTER FUNCTIONS *********
// ***********************************************************

static int Get_value_ptr (Bmi *self, const char *name, void **dest)
{
    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;

    // Use CSDMS standard names ONLY when role is 
    // INPUT_TO_BMI or OUPUT_TO_BMI

    //-------------------------------------
    //             ARRAY_LENGTH
    //-------------------------------------
    /*  0 num_sub_catchments
      1 max_atb_increments
      2 max_num_subcatchments
      3 max_time_delay_ordinates
    */

    if (strcmp (name, "num_sub_catchments") == 0) {
        *dest = (void*)&topmodel->num_sub_catchments;
    }
    else if (strcmp (name, "max_atb_increments") == 0) {
        *dest = (void*)&topmodel->max_atb_increments;
    }
    else if (strcmp (name, "max_num_subcatchments") == 0) {
        *dest = (void*)&topmodel->max_num_subcatchments;
    }
    else if (strcmp (name, "max_time_delay_ordinates") == 0) {
        *dest = (void*)&topmodel->max_time_delay_ordinates;
    }


    //-------------------------------------
    //             DIAGNOSTIC
    //-------------------------------------
    // sump
    else if (strcmp (name, "land_surface_water__domain_time_integral_of_precipitation_volume_flux") == 0) {
        *dest = (void*)&topmodel-> sump;
    }
    // sumae
    else if (strcmp (name, "land_surface_water__domain_time_integral_of_evaporation_volume_flux") == 0) {
        *dest = (void*)&topmodel-> sumae;
    }// sumq
    else if (strcmp (name, "land_surface_water__domain_time_integral_of_runoff_volume_flux") == 0) {
        *dest = (void*)&topmodel-> sumq;
    }
    // sumrz
    else if (strcmp (name, "soil_water__domain_root-zone_volume_deficit") == 0) {
        *dest = (void*)&topmodel-> sumrz;
    }
    // sumuz
    else if(strcmp (name, "soil_water__domain_unsaturated-zone_volume") == 0) {
        *dest = (void*)&topmodel-> sumuz;
    }

    //-------------------------------------
    //              FILE_OFFSET
    //-------------------------------------
    else if (strcmp (name, "control_fptr") == 0) {
        *dest = (void*)topmodel-> control_fptr;
    }
    else if (strcmp (name, "input_fptr") == 0) {
        *dest = (void*)topmodel-> input_fptr;
    }
    else if (strcmp (name, "subcat_fptr") == 0) {
        *dest = (void*)topmodel-> subcat_fptr;
    }
    else if (strcmp (name, "params_fptr") == 0) {
        *dest = (void*)topmodel-> params_fptr;
    }
    else if (strcmp (name, "output_fptr") == 0) {
        *dest = (void*)topmodel-> output_fptr;
    }
    else if (strcmp (name, "out_hyd_fptr") == 0) {
        *dest = (void*)topmodel-> out_hyd_fptr;
    }

    //-------------------------------------
    //             INFO_STRING
    //-------------------------------------
    else if (strcmp (name, "title") == 0) {
        *dest = (void*)(topmodel-> title);
    }
    else if (strcmp (name, "subcat") == 0) {
        *dest = (void*)topmodel-> subcat;
    }

    //-------------------------------------
    // INPUT_FROM_BMI - CSDMS Standard Names    
    //-------------------------------------
    // STANDALONE Note: 
    //      When TRUE/1 there are no bmi inputs being passed
    //      defs here speak to "scalar"  
    //      TODO: add logic to only apply these defs for framework runs 
    else if (strcmp (name, "water_potential_evaporation_flux") == 0) {
        *dest = (void*)topmodel-> pe;
        //*dest = (void*)(topmodel->pe + topmodel->current_time_step);
    }

    else if (strcmp (name, "atmosphere_water__liquid_equivalent_precipitation_rate") == 0) {
        *dest = (void*)topmodel->rain;
    }

    //-------------------------------------
    //           INPUT_FROM_FILE    
    //-------------------------------------
    else if (strcmp (name, "Qobs") == 0) {
        *dest = (void*)topmodel->Qobs;
    }

    //-------------------------------------
    //              OPTION    
    //-------------------------------------
    
    /*  0 yes_print_output
      1 imap
      2 infex
      3 stand_alone*/

    else if (strcmp (name, "yes_print_output") == 0) {
        *dest = (void*)&topmodel->yes_print_output;
    }
    else if (strcmp (name, "imap") == 0) {
        *dest = (void*)&topmodel->imap;
    }
    else if (strcmp (name, "infex") == 0) {
        *dest = (void*)&topmodel->infex;
    }
    else if (strcmp (name, "stand_alone") == 0) {
        *dest = (void*)&topmodel->stand_alone;
    }


    //-------------------------------------
    // OUTPUT_TO_BMI - CSDMS Standard Names    
    //-------------------------------------
    // Q[it]
    else if (strcmp (name, "land_surface_water__runoff_mass_flux") == 0) {
        *dest = (void*)topmodel-> Q;
    }
    // sbar
    else if (strcmp (name, "soil_water__domain_volume_deficit") == 0) {
        *dest = (void*)&topmodel-> sbar;
    }

    //-------------------------------------
    //           OUTPUT_TO_FILE    
    //-------------------------------------
    // p
    else if (strcmp (name, "atmosphere_water__domain_time_integral_of_rainfall_volume_flux") == 0) {
        *dest = (void*)&topmodel-> p;
    // ep    
    }
    else if (strcmp (name, "land_surface_water__potential_evaporation_volume_flux") == 0) {
        *dest = (void*)&topmodel-> ep;
    }
    // quz
    else if (strcmp (name, "soil_water_root-zone_unsat-zone_top__recharge_volume_flux") == 0) {
        *dest = (void*)&topmodel-> quz;
    }
    // qb
    else if (strcmp (name, "land_surface_water__baseflow_volume_flux") == 0) {
        *dest = (void*)&topmodel-> qb;
    }
    // qof
    else if (strcmp (name, "land_surface_water__domain_time_integral_of_overland_flow_volume_flux") == 0) {
        *dest = (void*)&topmodel-> qof;
    }
    // bal
    else if (strcmp (name, "land_surface_water__water_balance_volume") == 0) {
        *dest = (void*)&topmodel-> bal;
    }

    //-------------------------------------
    //            PARAMETER_FIXED    
    //-------------------------------------
    /*  0 num_channels
      1 num_topodex_values
      2 t0
      3 area
      4 num_delay
      5 num_time_delay_histo_ords
      6 szq
      7 tl
      8 time_delay_histogram
      9 dist_area_lnaotb
      10 lnaotb
      11 cum_dist_area_with_dist
      12 dist_from_outlet
      13 max_contrib_area*/
    else if (strcmp (name, "num_channels") == 0) {
        *dest = (void*)&topmodel-> num_channels;
    }
    else if (strcmp (name, "num_topodex_values") == 0) {
        *dest = (void*)&topmodel-> num_topodex_values;
    }
    else if (strcmp (name, "t0") == 0) {
        *dest = (void*)&topmodel-> t0;
    }
    else if (strcmp (name, "area") == 0) {
        *dest = (void*)&topmodel-> area;
    }
    else if (strcmp (name, "num_delay") == 0) {
        *dest = (void*)&topmodel-> num_delay;
    }
    else if (strcmp (name, "num_time_delay_histo_ords") == 0) {
        *dest = (void*)&topmodel-> num_time_delay_histo_ords;
    }
    else if (strcmp (name, "szq") == 0) {
        *dest = (void*)&topmodel-> szq;
    }
    else if (strcmp (name, "tl") == 0) {
        *dest = (void*)&topmodel-> tl;
    }
    else if (strcmp (name, "time_delay_histogram") == 0) {
        *dest = (void*)topmodel-> time_delay_histogram;
    }
    else if (strcmp (name, "dist_area_lnaotb") == 0) {
        *dest = (void*)topmodel-> dist_area_lnaotb;
    }
    else if (strcmp (name, "lnaotb") == 0) {
        *dest = (void*)topmodel-> lnaotb;
    }
    else if (strcmp (name, "cum_dist_area_with_dist") == 0) {
        *dest = (void*)topmodel-> cum_dist_area_with_dist;
    }
    else if (strcmp (name, "dist_from_outlet") == 0) {
        *dest = (void*)topmodel-> dist_from_outlet;
    }
    else if (strcmp (name, "max_contrib_area") == 0) {
        *dest = (void*)&topmodel-> max_contrib_area;
    }

    //-------------------------------------
    //         PARAMETER_ADJUSTABLE    
    //-------------------------------------
    /*  0 szm
      1 td
      2 srmax
      3 xk0
      4 hf
      5 dth*/
    // qof
    else if (strcmp (name, "szm") == 0) {
        *dest = (void*)&topmodel-> szm;
    }
    else if (strcmp (name, "td") == 0) {
        *dest = (void*)&topmodel-> td;
    }
    else if (strcmp (name, "srmax") == 0) {
        *dest = (void*)&topmodel-> srmax;
    }
    else if (strcmp (name, "xk0") == 0) {
        *dest = (void*)&topmodel-> xk0;
    }
    else if (strcmp (name, "hf") == 0) {
        *dest = (void*)&topmodel-> hf;
    }
    else if (strcmp (name, "dth") == 0) {
        *dest = (void*)&topmodel-> dth;
    }
    else if (strcmp (name, "qof") == 0) {
        *dest = (void*)&topmodel-> qof;
    }
    //-------------------------------------
    //                STATE    
    //-------------------------------------
    /*  0 Q0
      1 sr0
      2 contrib_area
      3 stor_unsat_zone
      4 deficit_root_zone
      5 deficit_local
      6 Qout
    */
    else if (strcmp (name, "Q0") == 0) {
        *dest = (void*)&topmodel-> Q0;
    }
    else if (strcmp (name, "sr0") == 0) {
        *dest = (void*)&topmodel-> sr0;
    }
    else if (strcmp (name, "contrib_area") == 0) {
        *dest = (void*)topmodel-> contrib_area;
    }
    else if (strcmp (name, "stor_unsat_zone") == 0) {
        *dest = (void*)topmodel-> stor_unsat_zone;
    }
    else if (strcmp (name, "deficit_root_zone") == 0) {
        *dest = (void*)topmodel-> deficit_root_zone;
    }
    else if (strcmp (name, "deficit_local") == 0) {
        *dest = (void*)topmodel-> deficit_local;
    }
    else if (strcmp (name, "Qout") == 0) {
        *dest = (void*)&topmodel-> Qout;
    }

    //-------------------------------------
    //              TIME_INFO    
    //-------------------------------------
    /*  0 dt
      1 nstep
      2 current_time_step
    */
    else if (strcmp (name, "dt") == 0) {
        *dest = (void*)&topmodel-> dt;
    }
    else if (strcmp (name, "nstep") == 0) {
        *dest = (void*)&topmodel-> nstep;
    }
    else if (strcmp (name, "current_time_step") == 0) {
        *dest = (void*)&topmodel-> current_time_step;
    }
/*    else {
        return BMI_FAILURE;
    }*/

    return BMI_SUCCESS;
}

static int Get_value_at_indices (Bmi *self, const char *name, void *dest, int * inds, int len)
{
    void *src = NULL;
    int itemsize = 0;

    if (self->get_value_ptr(self, name, &src) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_var_itemsize(self, name, &itemsize) == BMI_FAILURE)
        return BMI_FAILURE;

    { /* Copy the data */
        size_t i;
        size_t offset;
        char * ptr;
        for (i=0, ptr=(char*)dest; i<len; i++, ptr+=itemsize) {
            offset = inds[i] * itemsize;
            memcpy (ptr, (char*)src + offset, itemsize);
        }
    }

    return BMI_SUCCESS;
}

static int Get_value(Bmi * self, const char * name, void *dest)
{
    void *src = NULL;
    int nbytes = 0;

    if (self->get_value_ptr (self, name, &src) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_var_nbytes (self, name, &nbytes) == BMI_FAILURE)
        return BMI_FAILURE;

    memcpy(dest, src, nbytes);

    return BMI_SUCCESS;
}

static int Set_value (Bmi *self, const char *name, void *array)
{
    
/*    void * dest = NULL;
    int nbytes = 0;

    if (self->get_value_ptr(self, name, &dest) == BMI_FAILURE)
    return BMI_FAILURE;

    if (self->get_var_nbytes(self, name, &nbytes) == BMI_FAILURE)
    return BMI_FAILURE;

    memcpy (dest, array, nbytes);*/

    topmodel_model *topmodel;
    topmodel = (topmodel_model *) self->data;
    
    if (strcmp (name, "control_fptr") == 0) {
        topmodel->control_fptr = fopen("/dev/null", "w");
    }
    else if (strcmp (name, "input_fptr") == 0) {
        topmodel->input_fptr = fopen("/dev/null", "w");
    }
    else if (strcmp (name, "subcat_fptr") == 0) {
        topmodel->subcat_fptr = fopen("/dev/null", "w");
    }
    else if (strcmp (name, "params_fptr") == 0) {
        topmodel->params_fptr = fopen("/dev/null", "w");
    }
    else if (strcmp (name, "output_fptr") == 0) {
        topmodel->output_fptr = fopen("/dev/null", "w");
    }
    else if (strcmp (name, "out_hyd_fptr") == 0) {
        topmodel->out_hyd_fptr = fopen("/dev/null", "w");
    }
    else {

        void * dest = NULL;
        int nbytes = 0;

        if (self->get_value_ptr(self, name, &dest) == BMI_FAILURE)
            return BMI_FAILURE;

        if (self->get_var_nbytes(self, name, &nbytes) == BMI_FAILURE)
            return BMI_FAILURE;

        memcpy (dest, array, nbytes);
    }

    return BMI_SUCCESS;
}

static int Set_value_at_indices (Bmi *self, const char *name, int * inds, int len, void *src)
{
    void * to = NULL;
    int itemsize = 0;

    if (self->get_value_ptr (self, name, &to) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_var_itemsize(self, name, &itemsize) == BMI_FAILURE)
        return BMI_FAILURE;

    { /* Copy the data */
        size_t i;
        size_t offset;
        char * ptr;
        for (i=0, ptr=(char*)src; i<len; i++, ptr+=itemsize) {
            offset = inds[i] * itemsize;
            memcpy ((char*)to + offset, ptr, itemsize);
        }
    }
    return BMI_SUCCESS;
}
 

// ***********************************************************
// ************ BMI: MODEL INFORMATION FUNCTIONS *************
// ***********************************************************

static int Get_component_name (Bmi *self, char * name)
{
    strncpy (name, "TOPMODEL", BMI_MAX_COMPONENT_NAME);
    return BMI_SUCCESS;
}

// NEW BMI EXTENSION 
static int Get_bmi_version (Bmi *self, char * version)
{
    strncpy (version, "2.0_nGen_extension", BMI_MAX_VERSION_NAME);
    return BMI_SUCCESS;
}

// NEW BMI EXTENSION 
static int Get_model_var_roles (Bmi *self, char ** roles)
{
    for (int i = 0; i < VAR_ROLE_COUNT; i++) {
        strncpy (roles[i], model_var_roles[i], BMI_MAX_ROLE_NAME);
    }
    return BMI_SUCCESS;
}
static int Get_model_var_count (Bmi *self, const char *role, int *count)
{
    
    // If role is "all", don't filter just return VAR_NAME_COUNT
    if (strcmp(role, "all") == 0) {
        *count = VAR_NAME_COUNT;
        return BMI_SUCCESS;
    }    

    // This block code uses get_model_var_roles to check if *role
    // is valid, but maybe overkill?

/*    int is_role = 0;
    // Otherise, check if role is valid first
    // Get_model_var_roles would be "cleanest"?
    
    // Setup array size...
    char **role_list = NULL;
    role_list = (char**) malloc (sizeof(char *) * VAR_ROLE_COUNT);
    
    // Setup array element size... sigh
    for (int i=0; i<VAR_ROLE_COUNT; i++){
        role_list[i] = (char*) malloc (sizeof(char) * BMI_MAX_ROLE_NAME);
    }
    
    // Check if get_model_var_roles is okay
    int status = Get_model_var_roles(self, role_list);
    if (status == BMI_FAILURE){
        free(role_list);
        return BMI_FAILURE;
    }

    // Now finally, check if role exists yey
    for (int i=0; i<VAR_ROLE_COUNT; i++){
        if (strcmp(role, role_list[i]) == 0){
            is_role = 1;
        }
    }

    free(role_list);
    // Loop thru and count vars with this role
    if (is_role == 1){
        int this_count = 0;
        for (int i = 0; i < VAR_NAME_COUNT; i++) {
            if (strcmp(role, var_info[i].role) == 0) {
                this_count++;
            }    
        }
        *count = this_count;
        return BMI_SUCCESS;
    }
        */

    // Loop thru and count vars with this role
    int this_count = 0;
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp(role, var_info[i].role) == 0) {
            this_count++;
        }    
    }

    if (this_count > 0) {
        *count = this_count;
        return BMI_SUCCESS;
    }    
    
    // If we get here, it means the role wasn't recognized
    *count = 0;
    return BMI_SUCCESS;
    //return BMI_FAILURE;

}

static int Get_model_var_names (Bmi *self, const char *role, char **names)
{
    
    // If role is "all", don't filter just return all
    if (strcmp(role, "all") == 0) {
        for (int i=0; i<VAR_NAME_COUNT; i++){
            strncpy(names[i], var_info[i].name, BMI_MAX_VAR_NAME);
        }
    return BMI_SUCCESS;      
    }

    // This block code uses get_model_var_roles to check if *role
    // is valid, but maybe overkill?

/*    int is_role = 0;
    // Otherise, check if role is valid first
    
    // Setup array size...
    char **role_list = NULL;
    role_list = (char**) malloc (sizeof(char *) * VAR_ROLE_COUNT);
    
    // Setup array element size... sigh
    for (int i=0; i<VAR_ROLE_COUNT; i++){
        role_list[i] = (char*) malloc (sizeof(char) * BMI_MAX_ROLE_NAME);
    }
    
    // Check if get_model_var_roles is okay
    int status = Get_model_var_roles(self, role_list);
    if (status == BMI_FAILURE){
        free(role_list);
        return BMI_FAILURE;
    }

    // Now finally, check if role exists yey
    for (int i=0; i<VAR_ROLE_COUNT; i++){
        if (strcmp(role, role_list[i]) == 0){
            is_role = 1;
        }
    }

    // Loop thru and get var names with this role
    if (is_role == 1){
        int this_index = -1;
        for (int i = 0; i < VAR_NAME_COUNT; i++) {
            if (strcmp(role, var_info[i].role) == 0) {
                this_index++;
                strncpy (names[this_index], var_info[i].name, BMI_MAX_VAR_NAME);   
            }    
        }
        return BMI_SUCCESS;
    }*/

    // Loop thru and get var names with this role
    int this_index = -1;
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp(role, var_info[i].role) == 0) {
            this_index++;
            strncpy (names[this_index], var_info[i].name, BMI_MAX_VAR_NAME);   
        }    
    }

    return BMI_SUCCESS;
    
/*    if (this_index > -1) return BMI_SUCCESS;
        
    // If we get here, it means the role wasn't recognized
    return BMI_FAILURE;*/

}      

// This is now loops thru var_info struct
static int Get_input_item_count (Bmi *self, int * count)
{
/*    int input_count;
    int input_count_result = Get_model_var_count(self, &input_count, "input");
    if (input_count_result != BMI_SUCCESS) {
        return BMI_FAILURE;
    }
    *count = input_count;
    return BMI_SUCCESS;*/

    // Loop thru and count vars with this role = "input"
    int input_count = 0;
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp("input_from_bmi", var_info[i].role) == 0) {
            input_count++;
        }    
    }

    *count = input_count;
    return BMI_SUCCESS; //even if count = 0
  
}

static int Get_input_var_names (Bmi *self, char ** names)
{
/*    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        strncpy (names[i], input_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;*/

    // NEW BMI EXTENSION - this uses new get_model_var
/*    char * input_names;
    int input_names_result = Get_model_var_names(self, &&input_names, "input");
    if (input_names_result != BMI_SUCCESS) {
        return BMI_FAILURE;
    }
    **names = input_names;
    return BMI_SUCCESS; 
*/


    // now loops thru var_info struct
    int idx = -1; 
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp("input_from_bmi", var_info[i].role) == 0){
            idx++;
            strncpy (names[idx], var_info[i].name, BMI_MAX_VAR_NAME);
        }
    }    

    return BMI_SUCCESS;
}

// This is now loops thru var_info struct
static int Get_output_item_count (Bmi *self, int * count)
{
/*    int output_count;
    int output_count_result = Get_model_var_count(self, &output_count, "output");
    if (output_count_result != BMI_SUCCESS) {
        return BMI_FAILURE;
    }
    *count = output_count;
    return BMI_SUCCESS;*/

    // Loop thru and count vars with this role = "input"
    int output_count = 0;
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp("output_to_bmi", var_info[i].role) == 0) {
            output_count++;
        }    
    }

    *count = output_count;
    return BMI_SUCCESS; //even if count = 0
}

static int Get_output_var_names (Bmi *self, char ** names)
{
/*    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        strncpy (names[i], output_var_names[i], BMI_MAX_VAR_NAME);
    }*/
    
    // now loops thru var_info struct
    int idx = -1; 
    for (int i = 0; i < VAR_NAME_COUNT; i++) {
        if (strcmp("output_to_bmi", var_info[i].role) == 0){
            idx++;
            strncpy (names[idx], var_info[i].name, BMI_MAX_VAR_NAME);
        }
    }  

    return BMI_SUCCESS;
}


// ***********************************************************
// **************** BMI: MODEL GRID FUNCTIONS ****************
// ***********************************************************

/* Grid information */
static int Get_grid_rank (Bmi *self, int grid, int * rank)
{
    if (grid == 0) {
        *rank = 1;
        return BMI_SUCCESS;
    }
    else {
        *rank = -1;
        return BMI_FAILURE;
    }
}

static int Get_grid_size(Bmi *self, int grid, int * size)
{
    if (grid == 0) {
        *size = 1;
        return BMI_SUCCESS;
    }
    else {
        *size = -1;
        return BMI_FAILURE;
    }
}

static int Get_grid_type (Bmi *self, int grid, char * type)
{
    int status = BMI_FAILURE;

    if (grid == 0) {
        strncpy(type, "scalar", BMI_MAX_TYPE_NAME);
        status = BMI_SUCCESS;
    }
    else {
        type[0] = '\0';
        status = BMI_FAILURE;
    }
    return status;
}

/* Uniform rectilinear (grid type) */
static int Get_grid_shape(Bmi *self, int grid, int *shape)
{
    return BMI_FAILURE;
}

static int Get_grid_spacing(Bmi *self, int grid, double *spacing)
{
    return BMI_FAILURE;
}

static int Get_grid_origin(Bmi *self, int grid, double *origin)
{
    return BMI_FAILURE;
}

/* Non-uniform rectilinear, curvilinear (grid type)*/
static int Get_grid_x(Bmi *self, int grid, double *x)
{
    return BMI_FAILURE;
}

static int Get_grid_y(Bmi *self, int grid, double *y)
{
    return BMI_FAILURE;
}

static int Get_grid_z(Bmi *self, int grid, double *z)
{
    return BMI_FAILURE;
}

/*Unstructured (grid type)*/
static int Get_grid_node_count(Bmi *self, int grid, int *count)
{
    return BMI_FAILURE;
}

static int Get_grid_edge_count(Bmi *self, int grid, int *count)
{
    return BMI_FAILURE;
}

static int Get_grid_face_count(Bmi *self, int grid, int *count)
{
    return BMI_FAILURE;
}

static int Get_grid_edge_nodes(Bmi *self, int grid, int *edge_nodes)
{
    return BMI_FAILURE;
}

static int Get_grid_face_edges(Bmi *self, int grid, int *face_edges)
{
    return BMI_FAILURE;
}

static int Get_grid_face_nodes(Bmi *self, int grid, int *face_nodes)
{
    return BMI_FAILURE;
}

static int Get_grid_nodes_per_face(Bmi *self, int grid, int *nodes_per_face)
{
    return BMI_FAILURE;
}


topmodel_model * new_bmi_topmodel()  //(void)?
{
    topmodel_model *data;
    data = (topmodel_model*) malloc(sizeof(topmodel_model));
    //Init pointers to NULL
    data->Q = NULL;                    // simulated discharge
    data->Qobs = NULL;                 // observed discharge
    data->rain = NULL;                 // rainfall rate
    data->pe = NULL;                   // potential evapotranspiration
    data->contrib_area = NULL;         // contributing area
    data->stor_unsat_zone = NULL;      // storage in the unsat. zone
    data->deficit_root_zone = NULL;    // root zone storage deficit
    data->deficit_local = NULL;        // local storage deficit
    data->time_delay_histogram = NULL; // time lag of outflows due to channel routing
    data->dist_area_lnaotb = NULL;     // the distribution of area corresponding to ln(A/tanB) histo.
    data->lnaotb = NULL;               // these are the ln(a/tanB) values
    data->cum_dist_area_with_dist = NULL;  // channel cum. distr. of area with distance
    data->dist_from_outlet = NULL;     // distance from outlet to point on channel with area known
    return data;
}

Bmi* register_bmi_topmodel(Bmi *model)
{
    if (model) {
        model->data = (void*)new_bmi_topmodel();
        model->initialize = Initialize;
        model->update = Update;
        model->update_until = Update_until;
        model->finalize = Finalize;

        model->get_component_name = Get_component_name;
        model->get_bmi_version = Get_bmi_version;           //OWP CUSTOM
        model->get_model_var_count = Get_model_var_count;   //OWP CUSTOM
        model->get_model_var_roles = Get_model_var_roles;   //OWP CUSTOM
        model->get_model_var_names = Get_model_var_names;   //OWP CUSTOM
        model->get_input_item_count = Get_input_item_count;
        model->get_output_item_count = Get_output_item_count;
        model->get_input_var_names = Get_input_var_names;
        model->get_output_var_names = Get_output_var_names;

        model->get_var_grid = Get_var_grid;
        model->get_var_type = Get_var_type;
        model->get_var_itemsize = Get_var_itemsize;
        model->get_var_units = Get_var_units;
        model->get_var_nbytes = Get_var_nbytes;
        model->get_var_location = Get_var_location;

        model->get_var_index =      Get_var_index;          //OWP CUSTOM 
        model->get_var_role =       Get_var_role;           //OWP CUSTOM
        model->get_var_length =     Get_var_length;         //OWP CUSTOM

        model->get_current_time = Get_current_time;
        model->get_start_time = Get_start_time;
        model->get_end_time = Get_end_time;
        model->get_time_units = Get_time_units;
        model->get_time_step = Get_time_step;

        model->get_value = Get_value;
        model->get_value_ptr = Get_value_ptr;   
        model->get_value_at_indices = Get_value_at_indices;

        model->set_value = Set_value;
        model->set_value_at_indices = Set_value_at_indices;

        model->get_grid_size = Get_grid_size;
        model->get_grid_rank = Get_grid_rank;
        model->get_grid_type = Get_grid_type;

        // N/a for grid type scalar
        model->get_grid_shape = Get_grid_shape;
        model->get_grid_spacing = Get_grid_spacing;
        model->get_grid_origin = Get_grid_origin;

        // N/a for grid type scalar
        model->get_grid_x = Get_grid_x;
        model->get_grid_y = Get_grid_y;
        model->get_grid_z = Get_grid_z;

        // N/a for grid type scalar
        model->get_grid_node_count = Get_grid_node_count;
        model->get_grid_edge_count = Get_grid_edge_count;
        model->get_grid_face_count = Get_grid_face_count;
        model->get_grid_edge_nodes = Get_grid_edge_nodes;
        model->get_grid_face_edges = Get_grid_face_edges;
        model->get_grid_face_nodes = Get_grid_face_nodes;
        model->get_grid_nodes_per_face = Get_grid_nodes_per_face;

    }

    return model;
}
