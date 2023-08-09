#include "../include/topmodel.h" 
#include "../include/bmi.h" 
#include "../include/bmi_topmodel.h"

/* BMI Adaption: Max i/o file name length changed from 30 to 256 */
#define MAX_FILENAME_LENGTH 256
#define OUTPUT_VAR_NAME_COUNT 14
#define INPUT_VAR_NAME_COUNT 2
#define PARAM_VAR_NAME_COUNT 8
  
static const char *output_var_names[OUTPUT_VAR_NAME_COUNT] = {
        "Qout",
        // 11.18.2021 Edit: Just use the same input name as these two are ==  
        "atmosphere_water__liquid_equivalent_precipitation_rate_out",     //p
        "water_potential_evaporation_flux_out",                           //ep
        //"atmosphere_water__domain_time_integral_of_rainfall_volume_flux",   //p
        //"land_surface_water__potential_evaporation_volume_flux",            //ep
        "land_surface_water__runoff_mass_flux",                             //Q[it]
        "soil_water_root-zone_unsat-zone_top__recharge_volume_flux",        //qz
        "land_surface_water__baseflow_volume_flux",                         //qb
        "soil_water__domain_volume_deficit",                                //sbar
        "land_surface_water__domain_time_integral_of_overland_flow_volume_flux",    //qof
        "land_surface_water__domain_time_integral_of_precipitation_volume_flux",    //sump
        "land_surface_water__domain_time_integral_of_evaporation_volume_flux",      //sumae
        "land_surface_water__domain_time_integral_of_runoff_volume_flux",           //sumq
        "soil_water__domain_root-zone_volume_deficit",  //sumrz
        "soil_water__domain_unsaturated-zone_volume",   //sumuz
        "land_surface_water__water_balance_volume"      //bal
};

static const char *output_var_types[OUTPUT_VAR_NAME_COUNT] = {
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double",
        "double"
};

static const int output_var_item_count[OUTPUT_VAR_NAME_COUNT] = {
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1,
        1
};

static const char *output_var_units[OUTPUT_VAR_NAME_COUNT] = {
        "m h-1",
        "m h-1",
        "m h-1",
        "m h-1",
        "m h-1",
        "m h-1",
        "m",
        "m h-1",
        "m",
        "m",
        "m",
        "m",
        "m",
        "m"
};

static const int output_var_grids[OUTPUT_VAR_NAME_COUNT] = {
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0,
        0
};

static const char *output_var_locations[OUTPUT_VAR_NAME_COUNT] = {
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node",
        "node"
};

static const char *input_var_names[INPUT_VAR_NAME_COUNT] = {
        "atmosphere_water__liquid_equivalent_precipitation_rate",
        "water_potential_evaporation_flux"
};

static const char *input_var_types[INPUT_VAR_NAME_COUNT] = {
        "double",
        "double"
};

static const char *input_var_units[INPUT_VAR_NAME_COUNT] = {
        "m h-1",
        "m h-1"
};

static const int input_var_item_count[INPUT_VAR_NAME_COUNT] = {
        1,
        1
};

static const char input_var_grids[INPUT_VAR_NAME_COUNT] = {
        0,
        0
};

static const char *input_var_locations[INPUT_VAR_NAME_COUNT] = {
        "node",
        "node"
};

static const char *param_var_names[PARAM_VAR_NAME_COUNT] = {
    "t0",    // downslope transmissivity when the soil is just saturated to the surface
    "szm",   // exponential scaling parameter for the decline of transmissivity with increase in storage deficit (m)
    "td",    // unsaturated zone time delay per unit storage deficit (h)
    "srmax", // maximum root zone storage deficit (m)
    "sr0",   // initial root zone storage deficit below field capacity (m)
    "xk0",   // surface soil hydraulic conductivity (m/h)
    "chv",   // average channel velocity
    "rv"     // internal overland flow routing velocity
};

static const char *param_var_types[PARAM_VAR_NAME_COUNT] = {
    "double",
    "double",
    "double",
    "double",
    "double",
    "double",
    "double",
    "double"
};

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
        model->dt = 1;

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

/*    irof=0;
rex=0.0;
cumf=0.0;
max_contrib_area=0.0;
sump=0.0;
sumae=0.0;
sumq=0.0;
sae=0.0;*/


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

    if ((topmodel->stand_alone == FALSE) & (topmodel->yes_print_output == TRUE)){
        fprintf(topmodel->out_hyd_fptr,"%d %lf %lf\n",topmodel->current_time_step,topmodel->Qobs[1],topmodel->Q[1]);
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
      ((topmodel_model *)self->data)->dt = frac * dt;
      Update (self);
      ((topmodel_model *)self->data)->dt = dt;

    }

    return BMI_SUCCESS;
}

static int Finalize (Bmi *self)
{
    if (self){
        topmodel_model* model = (topmodel_model *)(self->data);

        if (model->yes_print_output == TRUE || TOPMODEL_DEBUG >= 1){        
            
            water_balance(model->output_fptr, model->yes_print_output,
                model->subcat,&model->bal, &model->sbar, &model->sump, 
                &model->sumae, &model->sumq, &model->sumrz, &model->sumuz);

            // this is technically needed, yes
            if(model->yes_print_output==TRUE){
                fprintf(model->output_fptr,"Maximum contributing area %12.5lf\n",model->max_contrib_area);
            }

            //-----------------------------------------------------------
            // When running in stand-alone mode, the original "results"
            // method should be called here in the Finalize() method,
            // not in Update_until(). It could also be called when in
            // framework-controlled mode.
            //-----------------------------------------------------------
            if (model->stand_alone == TRUE){
                results(model->output_fptr,model->out_hyd_fptr,model->nstep, 
                model->Qobs, model->Q, model->yes_print_output);                 
            }
        }    

        if( model->Q != NULL )
            free(model->Q);
        if( model->Qobs != NULL )
            free(model->Qobs);
        if( model->rain != NULL )
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
    // Check to see if in output array first
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
    }
    // Then check to see if in parameter array
    for (int i = 0; i < PARAM_VAR_NAME_COUNT; i++) {
        if (strcmp(name, param_var_names[i]) == 0) {
            strncpy(type, param_var_types[i], BMI_MAX_TYPE_NAME);
            return BMI_SUCCESS;
        }
    }
    // If we get here, it means the variable name wasn't recognized
    type[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_grid(Bmi *self, const char *name, int *grid)
{

    // Check to see if in output array first
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

    if (strcmp (type, "double") == 0) {
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
    else {
        *size = 0;
        return BMI_FAILURE;
    }
}

static int Get_var_location (Bmi *self, const char *name, char * location)
{
    // Check to see if in output array first
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
    }
    // If we get here, it means the variable name wasn't recognized
    location[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_units (Bmi *self, const char *name, char * units)
{
    // Check to see if in output array first
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
    }
    // If we get here, it means the variable name wasn't recognized
    units[0] = '\0';
    return BMI_FAILURE;
}

static int Get_var_nbytes (Bmi *self, const char *name, int * nbytes)
{
    int item_size;
    int item_size_result = Get_var_itemsize(self, name, &item_size);
    if (item_size_result != BMI_SUCCESS) {
        return BMI_FAILURE;
    }
    int item_count = -1;
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
        item_count = ((topmodel_model *) self->data)->nstep;

    *nbytes = item_size * item_count;
    return BMI_SUCCESS;
}


// ***********************************************************
// ********* BMI: VARIABLE GETTER & SETTER FUNCTIONS *********
// ***********************************************************

static int Get_value_ptr (Bmi *self, const char *name, void **dest)
{
    // Qout
    if (strcmp (name, "Qout") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> Qout;
        return BMI_SUCCESS;
    }
    // p
    if (strcmp (name, "atmosphere_water__liquid_equivalent_precipitation_rate_out") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> p;
        //*dest = (void*)&topmodel->rain[1]; Note: these are the same ==, either would work
        return BMI_SUCCESS;
    // ep    
    }
    if (strcmp (name, "water_potential_evaporation_flux_out") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> ep;
        //*dest = (void*)&topmodel-> pe[1]; Note: these are the same ==
        return BMI_SUCCESS;
    }
    // Q[it]
    if (strcmp (name, "land_surface_water__runoff_mass_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> Q[1];
        return BMI_SUCCESS;
    }
    // quz
    if (strcmp (name, "soil_water_root-zone_unsat-zone_top__recharge_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> quz;
        return BMI_SUCCESS;
    }
    // qb
    if (strcmp (name, "land_surface_water__baseflow_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> qb;
        return BMI_SUCCESS;
    }
    // sbar
    if (strcmp (name, "soil_water__domain_volume_deficit") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sbar;
        return BMI_SUCCESS;
    }
    // qof
    if (strcmp (name, "land_surface_water__domain_time_integral_of_overland_flow_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> qof;
        return BMI_SUCCESS;
    }
    // sump
    if (strcmp (name, "land_surface_water__domain_time_integral_of_precipitation_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sump;
        return BMI_SUCCESS;
    }
    // sumae
    if (strcmp (name, "land_surface_water__domain_time_integral_of_evaporation_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sumae;
        return BMI_SUCCESS;
    }// sumq
    if (strcmp (name, "land_surface_water__domain_time_integral_of_runoff_volume_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sumq;
        return BMI_SUCCESS;
    }
    // sumrz
    if (strcmp (name, "soil_water__domain_root-zone_volume_deficit") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sumrz;
        return BMI_SUCCESS;
    }
    // sumuz
    if (strcmp (name, "soil_water__domain_unsaturated-zone_volume") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sumuz;
        return BMI_SUCCESS;
    }
    // bal
    if (strcmp (name, "land_surface_water__water_balance_volume") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> bal;
        return BMI_SUCCESS;
    }
    // szm (parameter)
    if (strcmp (name, "szm") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> szm;
        return BMI_SUCCESS;
    }
    // td (parameter)
    if (strcmp (name, "td") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> td;
        return BMI_SUCCESS;
    }
    // srmax (parameter)
    if (strcmp (name, "srmax") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> srmax;
        return BMI_SUCCESS;
    }
    // sr0 (parameter)
    if (strcmp (name, "sr0") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> sr0;
        return BMI_SUCCESS;
    }
    // xk0 (parameter)
    if (strcmp (name, "xk0") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> xk0;
        return BMI_SUCCESS;
    }
    // t0 (parameter)
    if (strcmp (name, "t0") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> t0;
        return BMI_SUCCESS;
    }
    // chv (parameter)
    if (strcmp (name, "chv") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> chv;
        return BMI_SUCCESS;
    }
    // rv (parameter)
    if (strcmp (name, "rv") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> rv;
        return BMI_SUCCESS;
    }

    

    // STANDALONE Note: 
    //      When TRUE/1 there are no bmi inputs being passed
    //      defs here speak to "scalar"  
    //      TODO: add logic to only apply these defs for framework runs 
    if (strcmp (name, "water_potential_evaporation_flux") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> pe[1];
        //*dest = (void*)(topmodel->pe + topmodel->current_time_step);
        
        return BMI_SUCCESS;
    }

    if (strcmp (name, "atmosphere_water__liquid_equivalent_precipitation_rate") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel->rain[1];
        return BMI_SUCCESS;
    }

    return BMI_FAILURE;
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
    void * dest = NULL;
    int nbytes = 0;

    if (self->get_value_ptr(self, name, &dest) == BMI_FAILURE)
        return BMI_FAILURE;

    if (self->get_var_nbytes(self, name, &nbytes) == BMI_FAILURE)
        return BMI_FAILURE;

    memcpy (dest, array, nbytes);

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

    /* Adding code to allow for chv to update when calibrating BChoat 2023/08/07
     * chv and rv are calibratable params.
     * num_time_delay_histo_ords, time_delay_histogram, and num_delay are input
     * variables for topmod() */
    if (strcmp(name, "chv") == 0 || strcmp (name, "rv") == 0){

	double chv, rv;
	double dt, tch[11], rvdt, chvdt, t0dt, time, a1, sumar, sum, ar2, a2;
	double area, **time_delay_histogram, *dist_from_outlet;
	int i, j, ia, in, ir;
	int *num_time_delay_histo_ords, *num_delay, max_time_delay_ordinates;
	int num_channels, *cum_dist_area_with_dist;


	if((*time_delay_histogram)==NULL)
          {
	  d_alloc(time_delay_histogram, max_time_delay_ordinates);
          }

	
	rvdt=rv*dt;
        chvdt=chv*dt;

        tch[1]=dist_from_outlet[1]/chvdt;
        for(j=2;j<=num_channels;j++)
          {
          tch[j]=tch[1]+(dist_from_outlet[j]-dist_from_outlet[1])/rvdt;
          }
        (*num_time_delay_histo_ords)=(int)tch[num_channels];
        
        
        if((double)(*num_time_delay_histo_ords)<tch[num_channels]) 
           {
           (*num_time_delay_histo_ords)++;
           }
        
        (*num_delay)=(int)tch[1];
        (*num_time_delay_histo_ords)-=(*num_delay);
        for(ir=1;ir<=(*num_time_delay_histo_ords);ir++)
          {
          time=(double)(*num_delay)+(double)ir;
          if(time>tch[num_channels])
            {
            (*time_delay_histogram)[ir]=1.0;
            }
          else
            {
            for(j=2;j<=num_channels;j++)
              {
              if(time<=tch[j])
                {
                (*time_delay_histogram)[ir]=
                     cum_dist_area_with_dist[j-1]+
                        (cum_dist_area_with_dist[j]-cum_dist_area_with_dist[j-1])*
                                      (time-tch[j-1])/(tch[j]-tch[j-1]);
                break;  /* exits this for loop */
                }
              }
            }
          }
        a1=(*time_delay_histogram)[1];
        sumar=(*time_delay_histogram)[1];
        (*time_delay_histogram)[1]*=area;
        if((*num_time_delay_histo_ords)>1)
          {
          for(ir=2;ir<=(*num_time_delay_histo_ords);ir++)
            {
            a2=(*time_delay_histogram)[ir];
            (*time_delay_histogram)[ir]=a2-a1;
            a1=a2;
            sumar+=(*time_delay_histogram)[ir];
            (*time_delay_histogram)[ir]*=area;
            }
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

static int Get_input_item_count (Bmi *self, int * count)
{
    *count = INPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}

static int Get_input_var_names (Bmi *self, char ** names)
{
    for (int i = 0; i < INPUT_VAR_NAME_COUNT; i++) {
        strncpy (names[i], input_var_names[i], BMI_MAX_VAR_NAME);
    }
    return BMI_SUCCESS;
}

static int Get_output_item_count (Bmi *self, int * count)
{
    *count = OUTPUT_VAR_NAME_COUNT;
    return BMI_SUCCESS;
}

static int Get_output_var_names (Bmi *self, char ** names)
{
    for (int i = 0; i < OUTPUT_VAR_NAME_COUNT; i++) {
        strncpy (names[i], output_var_names[i], BMI_MAX_VAR_NAME);
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
    // data->chv = NULL;		       // average channel velocity
    // data->rv = NULL;		       // internal overland flow routing veoclity
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
