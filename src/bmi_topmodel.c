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
    "chv",   // average channel velcoity
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

    tread(model->subcat_fptr,model->output_fptr,model->subcat,&model->num_topodex_values,&model->num_channels,
        &model->area,&model->dist_area_lnaotb,&model->lnaotb,model->yes_print_output,
        &model->cum_dist_area_with_dist,&model->tl,&model->dist_from_outlet);

    fclose(model->subcat_fptr);

    init(model->params_fptr,model->output_fptr,model->subcat,model->num_channels,model->num_topodex_values,
        model->yes_print_output,model->area,&model->time_delay_histogram,model->cum_dist_area_with_dist,
        model->dt,model->tl,model->dist_from_outlet,&model->num_time_delay_histo_ords,&model->num_delay,
	&model->szm,&model->t0,&model->chv,&model->rv,&model->td, &model->srmax,
	&model->Q0,&model->sr0,&model->infex,&model->xk0,&model->hf,&model->dth,
	&model->stor_unsat_zone,&model->deficit_local,&model->deficit_root_zone,
        &model->szq,&model->Q,&model->sbar, &model->bal);
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

    
    ///////////////////////////////////
    //Handle calibratable parameters///
    ///////////////////////////////////

  
    // CHECK IF/WHICH CALIBRATABLE PARAMETERS TO UPDATE

    // DISCHARGE RELATED PARAMETERS

    // define array holding calibratable parameter names
    char *calibQParams[] = {"szm", "t0", "chv", "rv", "sr0"};

    // get number of strings to use for loop
    int numQParams = sizeof(calibQParams) / sizeof(calibQParams[0]);

    // check if name is == any calibratable parameter
    // create holder to check if name == any calibQParam
    int nameIsCalibQParam = 0;
    for (int i = 0; i<numQParams; i++){
 	if (strcmp(name, calibQParams[i]) == 0) {
	    nameIsCalibQParam = 1;
	    break;
        }
    }

    
    // WATER BALANCE RELATED PARAMETERS

    // define array holding calibratable parameter names
    char *calibWBParams[] = {"szm", "t0", "sr0"};

    // get number of strings to use for loop
    int numWBParams = sizeof(calibWBParams) / sizeof(calibWBParams[0]);

    // check if name is == any calibratable parameter
    // create holder to check if name == any calibWBParam
    int nameIsCalibWBParam = 0;
    for (int i = 0; i<numWBParams; i++){
 	if (strcmp(name, calibWBParams[i]) == 0) {
	    nameIsCalibWBParam = 1;
	    break;
        }
    }

    // if any calibratable variables were provided in realization file, then 
    // print an update with the values being used.
    // srmax and td do not need to be updated, but are printed to confirm their
    // presence in the realization.json file.
    if (nameIsCalibQParam || nameIsCalibWBParam || \
		    strcmp(name, "srmax") == 0 || strcmp(name, "td") == 0) {

        // instantiate topmodel as a pointer topmodel of type topmodel_model    
        topmodel_model *topmodel;
        // assign self->data to topmodel pointer
        topmodel = (topmodel_model *) self->data;

        printf("\n\n\nAT LEAST ONE OF THE FOLLOWING CALIBRATABLE PARAMETERS "
			"WAS PROVIDED IN THE REALIZATION.JSON FILE!\n");

	// print updated calibratable parameters
        printf("\n\nCalibratable parameters related to ET and recharge:\n");
	printf("srmax = %f\n", topmodel->srmax);
        printf("td = %f\n", topmodel->td);

	printf("\nCalibratable parameters related to discharge:\n");
        printf("chv = %f\n", topmodel->chv);
        printf("rv = %f\n", topmodel->rv);


	printf("\nCalibratable parameters related to water balance:\n");
        printf("szm = %f\n", topmodel->szm); 
	printf("sr0 = %f\n", topmodel->sr0);
        printf("t0 = %f\n\n\n\n", topmodel->t0);

    }

    // UPDATE APPROPRIATE PARAMETERS

    // DISCHARGE RELATED PARAMETERS

    // if name is a calibratable parameter, then update outputs
    if (nameIsCalibQParam) {

         // instantiate topmodel as a pointer topmodel of type topmodel_model    
        topmodel_model *topmodel;
        // assign self->data to topmodel pointer
        topmodel = (topmodel_model *) self->data;

        // declare variables
        double* tch; //FIXME put this in topmod struct and reuse it
        d_alloc(&tch, topmodel->num_channels);

	
     	// convert from distance/area to histogram ordinate form
        convert_dist_to_histords(topmodel->dist_from_outlet, topmodel->num_channels,
				&topmodel->chv, &topmodel->rv, topmodel->dt, tch);

        // calculate the time_delay_histogram
        calc_time_delay_histogram(topmodel->num_channels, 
	  			  topmodel->area, tch, 
 				  topmodel->cum_dist_area_with_dist, 
				  &topmodel->num_time_delay_histo_ords,
                  &topmodel->num_delay, &topmodel->time_delay_histogram);
        free(tch);
        // Reinitialise discharge array
        init_discharge_array(&topmodel->num_delay, &topmodel->Q0, topmodel->area, 
				&topmodel->num_time_delay_histo_ords, &topmodel->time_delay_histogram, 
				&topmodel->Q);	
    }


    // WATER BALANCE RELATED PARAMETERS

    // if name is a calibratable parameter, then update outputs
    if (nameIsCalibWBParam) {

        // instantiate topmodel as a pointer topmodel of type topmodel_model    
        topmodel_model *topmodel;
        // assign self->data to topmodel pointer
        topmodel = (topmodel_model *) self->data;

	
        // Initialize water balance and unsatrutaed storage and deficits
        init_water_balance(topmodel->num_topodex_values, 
					topmodel->dt, &topmodel->sr0, &topmodel->szm, 
					&topmodel->Q0, &topmodel->t0, topmodel->tl,
					&topmodel->stor_unsat_zone, &topmodel->szq, 
					&topmodel->deficit_local, &topmodel->deficit_root_zone, 
					&topmodel->sbar, &topmodel->bal);            
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
