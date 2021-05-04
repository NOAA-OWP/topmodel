#include "../include/topmodel.h" 
#include "../include/bmi.h" 
#include "../include/bmi_topmodel.h"

#define TOPMODEL_DEBUG 0

#define INPUT_VAR_NAME_COUNT 0
#define OUTPUT_VAR_NAME_COUNT 1

static const char *output_var_names[OUTPUT_VAR_NAME_COUNT] = {
        "Qout"
};

static const char *output_var_types[OUTPUT_VAR_NAME_COUNT] = {
        "double",
};

static const int output_var_item_count[OUTPUT_VAR_NAME_COUNT] = {
        1,
};

static const char *output_var_units[OUTPUT_VAR_NAME_COUNT] = {
        "m/h",
};

static const int output_var_grids[OUTPUT_VAR_NAME_COUNT] = {
        0,
};

static const char *output_var_locations[OUTPUT_VAR_NAME_COUNT] = {
        "node",
};

static const char *input_var_names[INPUT_VAR_NAME_COUNT] = {
};

static const char *input_var_types[INPUT_VAR_NAME_COUNT] = {
};

static const char *input_var_units[INPUT_VAR_NAME_COUNT] = {
};

static const int input_var_item_count[INPUT_VAR_NAME_COUNT] = {
};

static const char input_var_grids[INPUT_VAR_NAME_COUNT] = {
};

static const char *input_var_locations[INPUT_VAR_NAME_COUNT] = {
};

/** Count the number of values in a delimited string representing an array of values. */
static int count_delimited_values(char* string_val, char* delimiter)
{
    char *copy, *copy_to_free, *value;
    int count = 0;

    // Make duplicate to avoid changing original string
    // Then work on copy, but keep 2nd pointer to copy so that memory can be freed
    copy_to_free = copy = strdup(string_val);
    while ((value = strsep(&copy, delimiter)) != NULL)
        count++;
    free(copy_to_free);
    return count;
}

int read_file_line_counts(const char* file_name, int* line_count, int* max_line_length)
{

    /**line_count = 0;
    *max_line_length = 0;
    int current_line_length = 0;
    FILE* fp = fopen(file_name, "r");
    // Ensure exists
    if (fp == NULL) {
        printf("File does not exist.\n Failed in function read_file_line_counts\n");
        return -1;
    }
    int seen_non_whitespace = 0;
    char c;
    for (c = fgetc(fp); c != EOF; c = fgetc(fp)) {
        // keep track if this line has seen any char other than space or tab
        if (c != ' ' && c != '\t' && c != '\n')
            seen_non_whitespace++;
        // Update line count, reset non-whitespace count, adjust max_line_length (if needed), and reset current line count
        if (c == '\n') {
            *line_count += 1;
            seen_non_whitespace = 0;
            if (current_line_length > *max_line_length)
                *max_line_length = current_line_length;
            current_line_length = 0;
        }
        else {
            current_line_length += 1;
        }
    }
    fclose(fp);

    // If we saw some non-whitespace char on last line, assume last line didn't have its own \n, so count needs to be
    // incremented by 1.
    if (seen_non_whitespace > 0) {
        *line_count += 1;
    }

    // Before returning, increment the max line length by 1, since the \n will be on the line also.
    *max_line_length += 1;*/

    return 0;
}

int read_init_config(const char* config_file, topmodel_model* model) {
  
  char input_fname[30],subcat_fname[30],params_fname[30],output_fname[30];
  char out_hyd_fname[30];
  
  model-> max_atb_increments=30;
  model-> max_num_subcatchments=10;
  model-> max_time_delay_ordinates=20;

  if((model->control_fptr=fopen(config_file,"r"))==NULL)
    {printf("Can't open control file named topmod.run\n");exit(-9);}

  fgets(model->title,256,model->control_fptr);
  fscanf(model->control_fptr,"%s",input_fname);
  fscanf(model->control_fptr,"%s",subcat_fname);
  fscanf(model->control_fptr,"%s",params_fname);
  fscanf(model->control_fptr,"%s",output_fname);
  fscanf(model->control_fptr,"%s",out_hyd_fname);

  if((model->input_fptr=fopen("../data/inputs.dat","r"))==NULL)
    {printf("Can't open input file named %s\n",input_fname);exit(-9);}

  if((model->subcat_fptr=fopen("../data/subcat.dat","r"))==NULL)
    {printf("Can't open subcat file named %s\n",subcat_fname);exit(-9);}

  if((model->params_fptr=fopen("../data/params.dat","r"))==NULL)
    {printf("Can't open params file named %s\n",params_fname);exit(-9);}

  if((model->output_fptr=fopen("./topmod.out","w"))==NULL)
    {printf("Can't open output file named %s\n",output_fname);exit(-9);}

  if((model->out_hyd_fptr=fopen("./hyd.out","w"))==NULL)
    {printf("Can't open output file named %s\n",out_hyd_fname);exit(-9);}

}

int init_config(const char* config_file, topmodel_model* model)
{

    read_init_config(config_file,model);
    
    /*  READ IN nstep, DT and RAINFALL, PE, QOBS INPUTS */
    inputs(model->input_fptr, &model->nstep, &model->dt, &model->rain, &model->pe, &model->Qobs, &model->Q, &model->contrib_area);

    /*  READ IN SUBCATCHMENT TOPOGRAPHIC DATA */
    fscanf(model->subcat_fptr,"%d %d %d,",&model->num_sub_catchments,&model->imap,&model->yes_print_output);

    if(model->yes_print_output == TRUE)
        {fprintf(model->output_fptr,"%s\n",model->title);}

    tread(model->subcat_fptr,model->output_fptr,model->subcat,&model->num_topodex_values,&model->num_channels,
        &model->area,&model->dist_area_lnaotb,&model->lnaotb,model->yes_print_output,
        &model->cum_dist_area_with_dist,&model->tl,&model->dist_from_outlet,
        model->max_num_subcatchments,model->max_atb_increments);

    init(model->params_fptr,model->output_fptr,model->subcat,model->num_channels,model->num_topodex_values,
        model->yes_print_output,model->area,&model->time_delay_histogram,model->cum_dist_area_with_dist,
        model->dt,&model->szm,&model->t0,model->tl,model->dist_from_outlet,&model->td, &model->srmax,&model->Q0,&model->sr0,&model->infex,&model->xk0,&model->hf,
        &model->dth,&model->stor_unsat_zone,&model->deficit_local,&model->deficit_root_zone,
        &model->szq,model->Q,&model->sbar,model->max_atb_increments,model->max_time_delay_ordinates,
        &model->bal,&model->num_time_delay_histo_ords,&model->num_delay);

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
  Get_start_time(self, time);
  *time += (((topmodel_model *) self->data)->nstep * 
            ((topmodel_model *) self->data)->dt);
  return BMI_SUCCESS;
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
#if TOPMODEL_DEGUG > 1
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

    int config_read_result=init_config(cfg_file, topmodel);

    topmodel->current_time_step=0;
    
    topmodel->sump = 0.0;
    topmodel->sumae = 0.0;
    topmodel->sumq = 0.0;    

    return BMI_SUCCESS;
}

static int Update_frac (void * self, double f)
{ /* Implement this: Update for a fraction of a time step */
    return BMI_FAILURE;
}

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
        &topmodel->sbar,topmodel->num_delay,topmodel->current_time_step,
        &topmodel->sump,&topmodel->sumae,&topmodel->sumq);

    if (topmodel->yes_print_output == TRUE){
    results(topmodel->output_fptr,topmodel->out_hyd_fptr,topmodel->nstep, 
        topmodel->Qobs, topmodel->Q, topmodel->current_time_step);
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
    self->data = (void*)new_bmi_topmodel();

    fclose(model->control_fptr);
    fclose(model->input_fptr);
    fclose(model->subcat_fptr);
    fclose(model->params_fptr);
    fclose(model->output_fptr);
    fclose(model->out_hyd_fptr);
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
    if (strcmp (name, "Qout") == 0) {
        topmodel_model *topmodel;
        topmodel = (topmodel_model *) self->data;
        *dest = (void*)&topmodel-> Qout;
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


topmodel_model *
new_bmi_topmodel()  //(void)?
{
    topmodel_model *data;
    data = (topmodel_model*) malloc(sizeof(topmodel_model));

    // READ THESE FROM CONFIG FILE:
/*    data->time_step_size = 1;
    data->num_timesteps  = 1;
    data->current_time_step = 1;
*/
    return data;
}

Bmi* 
register_bmi_topmodel(Bmi *model)
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


//For inital development/testing
int 
main(void)
{
    int status = BMI_SUCCESS;
    Bmi *model = (Bmi *) malloc(sizeof(Bmi));

    register_bmi_topmodel(model);

    const char *cfg_file = "../data/topmod.run";
    double time = 0.0;
    double dt = 0.0;
    char units[BMI_MAX_UNITS_NAME];

   fprintf (stdout, "\n======= BEGIN UNIT TEST =======\n");

    // Test BMI initialize
    //model->initialize(model, cfg_file);
    {
      fprintf (stdout, "\nInitializing... ");
      status = model->initialize(model, cfg_file);
      if (status == BMI_FAILURE)
        return BMI_FAILURE;
      fprintf (stdout, "PASS\n");
    }
    
    // Test BMI get_start_time
    {
      fprintf (stdout, "Getting start time... ");
      status = model->get_start_time(model, &time);
      if (status == BMI_FAILURE)
        return BMI_FAILURE;
      fprintf(stdout, " %f ", time);
      fprintf (stdout, "PASS\n");
    }
    // Test BMI get_time_step
    {
      fprintf (stdout, "Getting time step... ");
      status = model->get_time_step(model, &dt);
      if (status == BMI_FAILURE)
        return BMI_FAILURE;
      fprintf(stdout, " %f ", dt);
      fprintf (stdout, "PASS\n");
    }
    // Test BMI get_time_units
    {
      fprintf (stdout, "Getting time units... ");
      status = model->get_time_units(model, units);
      if (status == BMI_FAILURE)
        return BMI_FAILURE;
      fprintf(stdout, " %s ", units);
      fprintf (stdout, "PASS\n");
    }

    for (int i=1;i<=100;i++) // shorter time loop for testing
    {
        // Test BMI update
        //model->update(model);
        {
        fprintf (stdout, "\nUpdating... ");
        status = model->update(model);
        if (status == BMI_FAILURE )
          return BMI_FAILURE;
        fprintf (stdout, "PASS\n");
        }
        
        // Test BMI get_current_time
        {
        fprintf (stdout, "Getting current time... ");
        status = model->get_current_time(model, &time);
        if (status == BMI_FAILURE )
          return BMI_FAILURE;
        fprintf(stdout, " %f ", time);
        fprintf (stdout, "PASS\n");
        }
        
        // Test BMI get_value* functions
        const char *var_name = "Qout";
        int len = 1;
        double *var = NULL;
        //size_t var_size;
        //var_size = sizeof(var) / sizeof(var[0]);
        //fprintf(stdout, "Var size = %d\n", var_size);
        { // get_value
            var = (double*) malloc (sizeof (double)*len);
            fprintf (stdout, "Getting value... ");
            status = model->get_value(model, var_name, var);
            //model->get_value(model, var_name, var);
            if (status == BMI_FAILURE )
                return BMI_FAILURE;
            fprintf (stdout, " %s %f PASS\n", var_name,var[0]);
            free(var);

        }
        { // get_value_at_indices
            int inds = 0;
            double *dest = NULL;
            dest = (double*) malloc (sizeof (double)*len);
            fprintf (stdout, "Getting value at indices... ");
            status = model->get_value_at_indices(model, var_name, dest, &inds, len);
            if (status == BMI_FAILURE )
                return BMI_FAILURE;
            fprintf (stdout, " %s %f PASS\n", var_name,dest[0]);
            free(dest);
        }
        { // get_value_ptr
            fprintf (stdout, "Getting value ptr... ");
            status = model->get_value_ptr(model, var_name, (void**)(&var));
            if (status == BMI_FAILURE )
                return BMI_FAILURE;
            fprintf (stdout, " %s %f PASS\n", var_name,var);
        }

    }

    //  model->update_until(model,950);
    
    // Test BMI get_component_name
    //model->get_component_name(model, name);
    {
        char name[BMI_MAX_COMPONENT_NAME];
        fprintf (stdout, "\nGetting component name... ");
        status = model->get_component_name(model, name);
        if (status == BMI_FAILURE )
          return BMI_FAILURE;
        fprintf (stdout, "%s PASS\n", name);
    }
    
    // Test BMI get_end_time
    {
        fprintf (stdout, "\nGetting end time... ");
        status = model->get_end_time(model,&time);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf(stdout, " %f ", time);
        fprintf (stdout, "PASS\n");
    }
    
    // Test BMI get_grid_* functions
    int grid = 0;
    int rank = 1;
    int size = 1;
    char type[BMI_MAX_COMPONENT_NAME];
    { // get_grid_rank
        fprintf (stdout, "\nGetting grid rank... ");
        status = model->get_grid_rank(model, grid, &rank);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "%i %i PASS\n", grid, rank);
    }
    { // get_grid_size
        fprintf (stdout, "Getting grid size... ");
        status = model->get_grid_size(model, grid, &size);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "%i %i PASS\n", grid, size);
    }
    { // get_grid_type
        fprintf (stdout, "Getting grid type... ");
        status = model->get_grid_type(model, grid, type);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf(stdout, " %i %s ", grid, type);
        fprintf (stdout, "PASS\n");
    }
    
    /* NOT IMPLEMENTED
    { // get_grid_edge_count
        fprintf (stdout, "Getting grid edge count... ");
        status = model->get_grid_edge_count(model, grid, &count);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "PASS\n");
    }
    { // get_grid_edge_nodes
        fprintf (stdout, "Getting grid edge nodes... ");
        status = model->get_grid_edge_nodes;
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "PASS\n");
    }
    { // get_grid_face_count
        fprintf (stdout, "Getting grid face count... ");
        status = model->get_grid_face_count;
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "PASS\n");
    }
    { // get_grid_face_edges
        fprintf (stdout, "Getting grid face edges... ");
        status = model->get_grid_face_edges;
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "PASS\n");
    }
    { // get_grid_face_nodes
        fprintf (stdout, "Getting grid face nodes... ");
        status = model->get_grid_face_nodes;
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "PASS\n");
    }
    { // get_grid_node_count
        fprintf (stdout, "Getting grid node count... ");
        status = model->get_grid_node_count;
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "PASS\n");
    }
    { // get_grid_nodes_per_face
        fprintf (stdout, "Getting grid nodes per face... ");
        status = model->get_grid_nodes_per_face;
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "PASS\n");
    }
    { // get_grid_origin
        fprintf (stdout, "Getting grid origin... ");
        status = model->get_grid_origin;
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "PASS\n");
    }
    { // get_grid_shape
         fprintf (stdout, "Getting grid shape... ");
         status = model->get_grid_shape;
         if (status == BMI_FAILURE )
             return BMI_FAILURE;
         fprintf (stdout, "PASS\n");
    }
    { // get_grid_spacing
         fprintf (stdout, "Getting grid spacing... ");
         status = model->get_grid_spacing;
         if (status == BMI_FAILURE )
             return BMI_FAILURE;
         fprintf (stdout, "PASS\n");
    }
     { // get_grid_x
         fprintf (stdout, "Getting grid x... ");
         status = model->get_grid_x;
         if (status == BMI_FAILURE )
             return BMI_FAILURE;
         fprintf (stdout, "PASS\n");
     }
     { // get_grid_y
         fprintf (stdout, "Getting grid y... ");
         status = model->get_grid_y;
         if (status == BMI_FAILURE )
             return BMI_FAILURE;
         fprintf (stdout, "PASS\n");
     }
     { // get_grid_z
         fprintf (stdout, "Getting grid z... ");
         status = model->get_grid_z;
         if (status == BMI_FAILURE )
             return BMI_FAILURE;
         fprintf (stdout, "PASS\n");
     }*/
     
    // Test BMI get_input/output_* functions
    int count = 0;
    char **names = NULL;
    int i;
    { // get_input_item_count
        fprintf (stdout, "\nGetting input item count... ");
        status = model->get_input_item_count(model, &count);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, " %d PASS\n", count);
    }
    { // get_input_var_names
        fprintf (stdout, "Getting input var names... ");
        names = (char**) malloc (sizeof(char *) * count);
        for (i=0; i<count; i++)
          names[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
        status = model->get_input_var_names(model, names);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        for (i=0; i<count; i++)
            fprintf (stdout, "%s", names[i]);
        fprintf (stdout, " PASS\n");
        free(names);
    }
    { // get_output_item_count
        fprintf (stdout, "Getting output item count... ");
        status = model->get_output_item_count(model, &count);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, " %d PASS\n", count);
    }
    { // get_output_var_names
        fprintf (stdout, "Getting output var names... ");
        names = (char**) malloc (sizeof(char *) * count);
        for (i=0; i<count; i++)
          names[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
        status = model->get_output_var_names(model, names);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        for (i=0; i<count; i++)
            fprintf (stdout, "%s", names[i]);
        fprintf (stdout, " PASS\n");
        free(names);
    }
    
    // Test BMI get_var_* functions
    { // get_var_grid
        const char *var_name = "Qout";
        fprintf (stdout, "\nGetting var grid... ");
        status = model->get_var_grid(model, var_name, &grid);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, " %s %i PASS\n", var_name,grid);
    }
    { // get_var_itemsize
        const char *var_name = "Qout";
        fprintf (stdout, "Getting var itemsize... ");
        status = model->get_var_itemsize(model, var_name, &size);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "%s %d PASS\n", var_name, size);
    }
    { // get_var_location
        const char *var_name = "Qout";
        char location[BMI_MAX_LOCATION_NAME];
        fprintf (stdout, "Getting var location... ");
        status = model->get_var_location(model, var_name, location);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "%s %s PASS\n", var_name, location);
    }
    { // get_var_units
        const char *var_name = "Qout";
        char units[BMI_MAX_UNITS_NAME];
        fprintf (stdout, "Getting var units... ");
        status = model->get_var_units(model, var_name, units);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "%s %s PASS\n", var_name, units);
    }
    { // get_var_nbytes
        const char *var_name = "Qout";
        int nbytes;
        fprintf (stdout, "Getting var nbytes... ");
        status = model->get_var_nbytes(model, var_name, &nbytes);
        if (status == BMI_FAILURE )
            return BMI_FAILURE;
        fprintf (stdout, "%s %d PASS\n", var_name, nbytes);
    }
    
    // Test BMI set_value
    double *var = NULL;
    const char *var_name = "Qout";
    int len = 1;
    var = (double*) malloc (sizeof (double)*len);
    // get_value
    model->get_value(model, var_name, var);
    fprintf(stdout, "\nQout = %f before set_value\n", var[0]);
    // set_value
    double *var_new = NULL;
    var_new = (double*) malloc (sizeof (double)*len);
    var_new[0] = 99.9;
    model->set_value(model, var_name, var_new);
    // get_value to see if changed
    model->get_value(model, var_name, var);
    fprintf(stdout, "Qout = %f after set_value\n", var[0]);
    free(var);
    free(var_new);

    // Test BMI set_value_at_indices
    int inds = 0;
    double *dest = NULL;
    dest = (double*) malloc (sizeof (double)*len);
    // get_value_at_indices
    model->get_value_at_indices(model, var_name, dest, &inds, len);
    fprintf(stdout, "\nQout = %f before set_value_at_indices\n", dest[0]);
    // set_value
    double *dest_new = NULL;
    dest_new = (double*) malloc (sizeof (double)*len);
    dest_new[0] = 11.1;
    model->set_value_at_indices(model, var_name, &inds, len, dest_new);
    // get_value_at_indices to see if changed
    model->get_value_at_indices(model, var_name, dest, &inds, len);
    fprintf(stdout, "Qout = %f after set_value_at_indices\n", dest[0]);
    free(dest);
    free(dest_new);
    
    /*int n_names;

    model->get_output_item_count(model, &n_names);

    names = (char**) malloc (sizeof(char *) * n_names);
    for (i=0; i<n_names; i++)
      names[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);

    model->get_output_var_names(model, names);

    for (i = 0; i<n_names; i++)
      printf ("Get Output Var Names: %s\n", names[i]);

    for (i=0; i<n_names; i++)
      free (names[i]);
    free (names);*/
    
    // Test BMI finalize
    //model->finalize(model);
    fprintf (stdout, "\nFinalizing... ");
    status = model->finalize(model);
    if (status == BMI_FAILURE )
      return BMI_FAILURE;
    fprintf (stdout, "PASS\n");

    return 0;
}
