//-----------------------------------------
// Notes: fbuffer.h is not included in:
//        /usr/local/include/msgpack.h
//-----------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // for strchr()
#include <msgpack.h>
#include "msgpack/fbuffer.h"        // See note above
#include "../include/topmodel.h"
#include "../include/bmi.h"
#include "../include/bmi_topmodel.h"

//-----------------------------------
// Make sure these are large enough
//-----------------------------------
static const int BUFFER_SIZE = 65536;
static const int UNPACKED_BUFFER_SIZE = 131072;
// static const int BUFFER_SIZE = 1024;
// static const int UNPACKED_BUFFER_SIZE = 2048;

/*
These functions are meant to provide a "framework utility"
that can serialize, save to file, and then deserialize all
of the state variables of a model written in C.

The function "serialize()" uses new BMI functions to retrieve
the model's state variables called:  get_state_var_ptrs(),
get_state_var_names(), get_state_var_types(), and
get_state_var_sizes().

The function "deserialize_to_state()" deserializes the saved model
(from a file) and then uses the new BMI function set_state_var()
to set all state variable values into a new instance of the model.
This new instance could be running on another node.

The function "compare_states()" compares all of the state
variables of two instances of the given model, to confirm that
they are identical after calling "deserialize_to_state()".

These new functions could be used for the problem of
load balancing or for recovery after hardware failures.

See this documentation for msgpack:
https://github.com/msgpack/msgpack-c/wiki/v2_0_c_overview

See this msgpack example:
https://blog.gypsyengineer.com/en/security/msgpack-fuzzing.html
*/

//-----------------------------------------------------------------------
int serialize(Bmi* model1, const char *ser_file) {

    int n_state_vars;
    model1->get_state_var_count(model1, &n_state_vars);
    
    FILE *fp = fopen(ser_file, "w+");
    int i, j, n_bytes;  
    void *ptr_list[ n_state_vars ];
    unsigned int size, sizes[ n_state_vars ];
    int verbose = 1;
    int    i_val;
    float  f_val;
    double d_val;
  
    char **names = NULL;
    names = (char**) malloc (sizeof(char *) * n_state_vars);
    for (i=0; i<n_state_vars; i++){
        names[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
    }

    char *type;
    char **types = NULL;
    types = (char**) malloc (sizeof(char *) * n_state_vars);  
    for (i=0; i<n_state_vars; i++){
        types[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
    }

    //--------------------------------------------------------------
    // Get required information on the model's state variables
    //--------------------------------------------------------------  
    if (verbose){ puts("Calling BMI.get_state_var_names()..."); }
    model1->get_state_var_names(model1, names);

    if (verbose){ puts("Calling BMI.get_state_var_types()..."); }
    model1->get_state_var_types(model1, types);

    if (verbose){ puts("Calling BMI.get_state_var_sizes()..."); }
    model1->get_state_var_sizes(model1, sizes);      

    if (verbose){ puts("Calling BMI.get_state_var_ptrs()..."); }
    model1->get_state_var_ptrs(model1, ptr_list);

    //--------------------------------------------
    // Prepare to write serialized state to file
    // msgpack_fbuffer_write needs fbuffer.h
    //--------------------------------------------
    msgpack_packer pk;
    msgpack_packer_init(&pk, fp, msgpack_fbuffer_write);

    //----------------------------------------------
    // Prepare to write serialized state to buffer
    //----------------------------------------------
    // msgpack_sbuffer* buffer = msgpack_sbuffer_new();
    // msgpack_packer* pk = msgpack_packer_new(buffer, msgpack_sbuffer_write);
  
    //------------------------------------------------------
    // Note: strings are serialized with 2 commands, like:
    //------------------------------------------------------
    // msgpack_pack_str(&pk, 7);
    // msgpack_pack_str_body(&pk, "example", 7);

    //--------------------------------------
    // Note: booleans are serialized like:
    //--------------------------------------
    // msgpack_pack_true(&pk);
    // msgpack_pack_false(&pk);

    //----------------------------------------------
    // Dereference the pointers & serialize values
    // Note that we aren't using the names here.
    //---------------------------------------------- 
    if (verbose){
        puts("Serializing the model state variables...");
        printf("  Number of state vars = %d\n", n_state_vars);
    } 
    for (i=0; i<n_state_vars; i++){
        size = sizes[i];
        type = types[i];
        printf("  i = %d, type = %s, size = %u\n", i, type, size );

        if (ptr_list[i] == NULL){
            if (verbose){
                printf("  NULL pointer encountered for i = %d.\n", i); }
        } 

        //-------------------------------
        // Does type name contain "*" ?
        //-------------------------------
        // printf("BEFORE type = %s\n", type);
        if (strchr(type, '*') != NULL){
            type[strlen(type)-1] = '\0';  }   // remove last char
        // printf("AFTER type  = %s\n", type);
        // printf("\n");

        //---------------------------------------------
        // Is this state var a single value or array?
        //---------------------------------------------
        if (size == 1){
            if (strcmp(type, "int") == 0){
                msgpack_pack_int(&pk, *(int *)ptr_list[i]);
            } else if (strcmp(type, "float") == 0){
                msgpack_pack_float(&pk, *(float *)ptr_list[i]);  
            } else if (strcmp(type, "double") == 0){
                msgpack_pack_double(&pk, *(double *)ptr_list[i]);
            } else if (strcmp(type, "string") == 0){
                // Note:  Need ptr_list[i] without * here.
                msgpack_pack_str_body(&pk, ptr_list[i], size);   
            } else if (strcmp(type, "FILE") == 0){
                // nil is an object pointer to nothing
                msgpack_pack_nil(&pk);  // Need something; will this work?
            } else{
                printf("  Unknown type = %s", types[i] );
                msgpack_pack_nil(&pk);  // Need something; will this work?
            }
        } else{
            //---------------------------------------
            // This state var is an array or string
            //---------------------------------------
            if (strcmp(type, "string") == 0){
                msgpack_pack_str(&pk, size);
            } else if (strcmp(type, "FILE") != 0){
                msgpack_pack_array(&pk, size);
                // n_bytes = size * sizeof( double );
                // msgpack_pack_bin(&pk, n_bytes); 
            }

            //-----------------------------------------------------
            // NOTE! Typecast ptr first, then add offset,
            //       into array, then dereference the ptr.
            //       CORRECT:    *( ((double *)ptr) + j)
            //       INCORRECT:  *( (double *)(ptr + j))
            //       INCORRECT:  *( (double *)ptr + j)  ??
            //       INCORRECT:  *(double *)ptr + j 
            //---------------------------------------------
            if (strcmp(type, "int") == 0){
                for (j=0; j<size; j++){
                    i_val = *( ((int *)ptr_list[i]) + j);
                        msgpack_pack_int(&pk, i_val); }
            } else if (strcmp(type, "float") == 0){
                for (j=0; j<size; j++){
                    f_val = *( ((float *)ptr_list[i]) + j);
                    msgpack_pack_float(&pk, f_val); }
            } else if (strcmp(type, "double") == 0){
                for (j=0; j<size; j++){
                    d_val = *( ((double *)ptr_list[i]) + j);
                    msgpack_pack_double(&pk, d_val); }
            } else if (strcmp(type, "string") == 0){
                // Note:  Need ptr_list[i] without * here.
                msgpack_pack_str_body(&pk, ptr_list[i], size);   
            } else if (strcmp(type, "FILE") == 0){
                for (j=0; j<size; j++){
                    // nil is an object pointer to nothing
                     msgpack_pack_nil(&pk); } // Need something; will this work?
            } else{
                printf("  Unknown type = %s", types[i] );
                msgpack_pack_nil(&pk);  // Need something; will this work?
            }
        }
    }
        
    //---------------------------------
    // If writing to file, close file
    //---------------------------------
    if (verbose){ puts("Closing output file..."); }
    fclose(fp);

    //------------------------------------  
    // If writing to buffer, free memory
    //------------------------------------
    // msgpack_sbuffer_free(buffer);
    // msgpack_packer_free(pk);
  
    //--------------------------------   
    // Free memory for string arrays
    //--------------------------------
    if (verbose){ puts("Freeing memory..."); }
    for (i=0; i<n_state_vars; i++){
        free (names[i]);
        free (types[i]);
    }
    free (names);
    free (types);

    if (verbose){ puts("Finished serializing."); puts(""); }
    return 0;
}
//--------------------------------------------------------------------
int deserialize_to_state(const char *ser_file, Bmi* model2, int print_obj) {

    //-----------------------------------------------------------
    // This online reference shows how to unpack the serialized
    // buffer one object at a time, as done here.
    //-----------------------------------------------------------
    // https://github.com/msgpack/msgpack-c/blob/c_master/
    //         example/user_buffer_unpack.c

    //-----------------------------------------------------------
    // This online reference shows how to save the serialized
    // buffer to a file and read it back in, also done here.
    //-----------------------------------------------------------
    // https://blog.gypsyengineer.com/en/security/msgpack-fuzzing.html
    
    //------------------------
    // Additional references
    //--------------------------------------------------------------------       
    // https://stackoverflow.com/questions/15393838/
    //         how-do-i-unpack-and-extract-data-properly-using-msgpack-c
    // https://stackoverflow.com/questions/12431441/messagepack-c-api/
    //         12581029#12581029
    //--------------------------------------------------------------------
    int verbose   = 1;
    // int print_obj = 1;
    int n_state_vars;
    model2->get_state_var_count(model2, &n_state_vars);
 
    unsigned int sizes[ n_state_vars ], size;
    model2->get_state_var_sizes(model2, sizes);
    int     i_val, *i_arr, j;
    float   f_val, *f_arr;
    double  d_val, *d_arr;
    void    *ptr;
        
    char *type, *sval;
    char **types = NULL;
    types = (char**) malloc (sizeof(char *) * n_state_vars);  
    for (int j=0; j<n_state_vars; j++){
        types[j] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
    }
    model2->get_state_var_types(model2, types);

    char inbuffer[BUFFER_SIZE]; 
    FILE *fp = fopen(ser_file, "rb");
    int i = 0;
    size_t off = 0;
    size_t len = 0;
    char unpacked_buffer[UNPACKED_BUFFER_SIZE];
    msgpack_unpacked unpacked;
    msgpack_unpack_return ret;
    msgpack_unpacked_init(&unpacked);
    if (verbose){ puts("Deserializing source model state vars..."); } 
    
    //------------------------------------------------------------
    // In online ref 1, buffer is passed as an argument (buf).
    // In online ref 2, buffer is read from ser_file (inbuffer).
    //------------------------------------------------------------
    // In online ref 1, len is passed as an argument.
    // In online ref 2, len=read is return value from fread.
    //------------------------------------------------------------
    // Online ref 1: "result" = Online ref 2: "unpacked".
    // Online ref 1: "buf" = Online ref 2: "inbuffer"  ????
    // Online ref 1: "len" = Online ref 2: "read"
    //------------------------------------------------------------
    len = fread(inbuffer, sizeof(char), BUFFER_SIZE, fp); 
    ret = msgpack_unpack_next(&unpacked, inbuffer, len, &off);

    while (ret == MSGPACK_UNPACK_SUCCESS) {
        msgpack_object obj = unpacked.data;

        size = sizes[i];
        type = types[i];
        // printf("type = %s\n", type);
        if (strchr(type, '*') != NULL){
            type[strlen(type)-1] = '\0';  // remove last char
        }

        if (size == 1){
            //--------------------------------------------
            // Note:  This does not work, even though we
            //        typecast in set_state_var().
            //--------------------------------------------
            // ptr = &obj;
            // model2->set_state_var(model2, ptr, i );
            //--------------------------------------------                  
            if (strcmp(type, "int") == 0){
                i_val = (int)obj.via.i64;
                model2->set_state_var(model2, &i_val, i );
            } else if (strcmp(type, "float") == 0){        
                f_val = (float)obj.via.f64;
                model2->set_state_var(model2, &f_val, i );
            } else if (strcmp(type, "double") == 0){   
                d_val = (double)obj.via.f64;
                model2->set_state_var(model2, &d_val, i );
            } else if (strcmp(type, "string") == 0){
                model2->set_state_var(model2, &obj, i );
                //-------------------------------------------
                // Next 2 lines don't work
                // sval = (char*)(obj.via.str);
                // model2->set_state_var(model2, sval, i );
                //-------------------------------------------
                // Next 2 lines don't work either
                //sval = obj.via.str;
                //model2->set_state_var(model2, sval, i );
            } else{
                 model2->set_state_var(model2, &obj, i );
            }
        } else{
            //--------------------------------------------------
            // Copy values from msgpack array object into an
            // array of the correct type, then pass pointer to
            // that array to set_state_var(). (This works.)
            //--------------------------------------------------
            // Couldn't figure out a clean way to access the
            // msgpack array elements with pointer math. This
            // seems to be because it allows mixed types.
            // Options:  via.u64, via.i64, via.f64, via.str.
            //--------------------------------------------------
            if (strcmp(type, "string") == 0){
                // Works for title & subcat strings in TOPMODEL.
                ptr = obj.via.array.ptr;
                model2->set_state_var(model2, ptr, i );
            } else if (strcmp(type, "int") == 0){
                i_arr = (int*) malloc(size * sizeof( int ));
                for (int j=0; j<size; j++){
                    i_arr[j] = obj.via.array.ptr[j].via.i64; }
                model2->set_state_var(model2, i_arr, i );
                free(i_arr);
            } else if (strcmp(type, "float") == 0){
                f_arr = (float*) malloc(size * sizeof( float ));
                for (int j=0; j<size; j++){
                    f_arr[j] = obj.via.array.ptr[j].via.f64; }
                model2->set_state_var(model2, f_arr, i );
                free(f_arr);
            } else if (strcmp(type, "double") == 0){
                d_arr = (double*) malloc(size * sizeof( double ));
                for (int j=0; j<size; j++){
                    d_arr[j] = obj.via.array.ptr[j].via.f64; }
                model2->set_state_var(model2, d_arr, i );
                free(d_arr);
            }
            
            // For testing
            //printf("### obj.via.array.ptr[0].via.f64 = %f\n", obj.via.array.ptr[0].via.f64);
            //printf("### obj.via.array.ptr[1].via.f64 = %f\n", obj.via.array.ptr[1].via.f64);
            //printf("### obj.via.array.ptr[2].via.f64 = %f\n", obj.via.array.ptr[2].via.f64);
            //printf("\n");
        }
        
            //------------------------------------------------------------
            // Failed attempts to access a single pointer to an array in
            // the msgpack_array structure, to pass to set_state_var().
            // Note that typecasting occurs in set_state_var().
            //------------------------------------------------------------
//          if (strcmp(type, "double") == 0){
//              ptr = obj.via.array.ptr;
//              model2->set_state_var(model2, ptr, i ); }
//          //-----------------------------------------------
//          if (strcmp(type, "double") == 0){
//              ptr = &obj;
//              model2->set_state_var(model2, ptr, i ); }
//          //-----------------------------------------------
//          if (strcmp(type, "double") == 0){
//              ptr = obj.via.array.ptr[0];
//              model2->set_state_var(model2, ptr, i ); }
//          //-----------------------------------------------
//          if (strcmp(type, "double") == 0){
//              d_val = obj.via.array.ptr[0].via.f64;
//              ptr   = &d_val;
//              model2->set_state_var(model2, ptr, i ); }
//          //-----------------------------------------------  
        
        if (print_obj){
            printf("Object no %d:\n", i);
            msgpack_object_print(stdout, obj);
            printf("\n");
              
            //------------------------------
            // This just prints obj again
            //------------------------------
            // msgpack_object_print_buffer(unpacked_buffer, UNPACKED_BUFFER_SIZE, obj);
            // printf("%s\n", unpacked_buffer);        
        }       
        //-----------------------------------------------------
        i++;
        ret = msgpack_unpack_next(&unpacked, inbuffer, len, &off);
    }   // end of while loop

    //--------------------------------------------------------
    // Did we unpack the expected number of state variables?
    //--------------------------------------------------------
    if (i < n_state_vars){
        printf("WARNING: Expected %d state variables \n", n_state_vars);
        printf("         But unpacked only %d vars. \n", i);
        printf("         BUFFER_SIZE may be too small.");
        printf("");
    } else{
        printf("Unpacked %d state variables.\n", i);
        printf("");     
    }

    //if (ret == MSGPACK_UNPACK_CONTINUE) {
    //    printf("Every msgpack_object in the buffer was processed.\n");
    //}
    //else if (ret == MSGPACK_UNPACK_PARSE_ERROR) {
    //    printf("The data in the buffer has an invalid format.\n");
    //}

    msgpack_unpacked_destroy(&unpacked);
    fclose(fp);  // Try moving this up to just after fread.

    //--------------------------------   
    // Free memory for string arrays
    //--------------------------------
    if (verbose){ puts("Freeing memory...");}
    for (i=0; i<n_state_vars; i++){
        // free (names[i]);
        free (types[i]);
    }
    // free (names);
    free (types);
    
    if (verbose){ puts("Finished deserializing."); puts("");}
    return 0;
}

//------------------------------------------------------------------------
int compare_states(Bmi* model1, Bmi* model2){

    int verbose = 1;
    int    i, j, match, n_state_vars;
    int    err_count = 0;
    int    i_val1, i_val2;
   float  f_val1, f_val2;
    double d_val1, d_val2;
    char   c_val1, c_val2;

    //--------------------------------------
    // Get total number of state variables
    //--------------------------------------
   model1->get_state_var_count(model1, &n_state_vars); 

    //----------------------------------------
    // Get the state variable internal names
    //----------------------------------------
    char **names = NULL;
    names = (char**) malloc (sizeof(char *) * n_state_vars);
    for (i=0; i<n_state_vars; i++){
        names[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
    }
    model1->get_state_var_names(model1, names);
  
    //------------------------------------  
    // Get the state variable data types
    //------------------------------------
    char *type;
    char **types = NULL;
    types = (char**) malloc (sizeof(char *) * n_state_vars);  
    for (i=0; i<n_state_vars; i++){
        types[i] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
    }
    model1->get_state_var_types(model1, types);

    //-------------------------------  
    // Get the state variable sizes
    //-------------------------------
    unsigned int size, sizes[ n_state_vars ];  
    model1->get_state_var_sizes(model1, sizes);

   //-------------------------------------        
    // Get pointers to Model 1 state vars
    //-------------------------------------
    void *ptr_list1[ n_state_vars ];
    model1->get_state_var_ptrs(model1, ptr_list1);

    //-------------------------------------
    // Get pointers to Model 2 state vars
    //-------------------------------------  
    void *ptr_list2[ n_state_vars ];
    model2->get_state_var_ptrs(model2, ptr_list2);

    //--------------------------------
    // Loop over all state variables
    //--------------------------------
    for (int i=0; i<n_state_vars; i++){
        match = 1;
        type  = types[i];
        size  = sizes[i];
        // printf("i = %d\n", i);  //##############
      
        //-------------------------------
        // Does type name contain "*" ?
        //-------------------------------
        if (strchr(type, '*') != NULL){
            type[strlen(type)-1] = '\0';  }   // remove last char
          
        //----------------------------------------------
        // NOTE! Typecast ptr first, then add offset,
        //       into array, then dereference the ptr.
        //       CORRECT:    *( ((double *)ptr) + j)
        //       INCORRECT:  *( (double *)(ptr + j))
        //       INCORRECT:  *( (double *)ptr + j)  ??
        //       INCORRECT:  *(double *)ptr + j 
        //---------------------------------------------
        if (strcmp(type, "int") == 0){
            for (j=0; j<size; j++){
                i_val1 = *( ((int *)ptr_list1[i]) + j);
                i_val2 = *( ((int *)ptr_list2[i]) + j);
                if (i_val1 != i_val2){
                    printf("Mismatch: i = %d, j = %d\n", i, j);
                    printf("i_val1 = %d, i_val2 = %d\n", i_val1, i_val2);
                    printf("var_name = %s\n\n", names[i]);
                    match = 0;}
              }
        } else if (strcmp(type, "float") == 0){
            for (j=0; j<size; j++){
                f_val1 = *( ((float *)ptr_list1[i]) + j);
                f_val2 = *( ((float *)ptr_list2[i]) + j);
                if (f_val1 != f_val2){
                    printf("Mismatch: i = %d, j = %d\n", i, j);
                    printf("f_val1 = %f, f_val2 = %f\n", f_val1, f_val2);
                    printf("var_name = %s\n\n", names[i]);
                    match = 0; }
            }
        } else if (strcmp(type, "double") == 0){
            for (j=0; j<size; j++){
                d_val1 = *( ((double *)ptr_list1[i]) + j);
                d_val2 = *( ((double *)ptr_list2[i]) + j);
                if (d_val1 != d_val2){
                    printf("Mismatch: i = %d, j = %d\n", i, j);
                    printf("d_val1 = %f, d_val2 = %f\n", d_val1, d_val2);
                    printf("var_name = %s\n\n", names[i]);
                    match = 0; }
            }
        } else if (strcmp(type, "string") == 0){
              if (strcmp(ptr_list1[i], ptr_list2[i]) != 0){
                    printf("Mismatch: i = %d\n", i);
                    printf("str1 = %s\n", ptr_list1[i]);
                    printf("str2 = %s\n", ptr_list2[i]);                          
                    printf("var_name = %s\n\n", names[i]);
                    match = 0;
              }
        } else if (strcmp(type, "FILE") == 0){
              j = 0;  // do nothing 
        } else{
              printf("  Unknown type = %s", types[i] );
        }

        //---------------------------------      
        // Did the state variables match?
        //---------------------------------
        if (match == 0){
            err_count++;
        } else{
            printf("Match: i = %d, name = %s\n", i, names[i]) ;        
        }
    }

    //--------------------------------   
    // Free memory for string arrays
    //--------------------------------
    if (verbose){ puts("Freeing memory...");}
    for (i=0; i<n_state_vars; i++){
        free (names[i]);
        free (types[i]);
    }
    free (names);
    free (types);
  
    printf("Number of state variable differences between\n");
    printf("   the two model states = %d\n", err_count);
    printf("Finished comparing model states.\n\n");

    return 0;
}

//------------------------------------------------------------------------


