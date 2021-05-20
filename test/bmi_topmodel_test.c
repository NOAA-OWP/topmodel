#include "../include/topmodel.h" 
#include "../include/bmi.h" 
#include "../include/bmi_topmodel.h"

//For inital development/testing
int 
main(void)
{
    int status = BMI_SUCCESS;
    Bmi *model = (Bmi *) malloc(sizeof(Bmi));

    register_bmi_topmodel(model);

    const char *cfg_file = "./data/topmod.run";
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
