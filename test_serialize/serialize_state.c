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

The function "deserialize()" deserializes the saved model
(from a file) and then uses the new BMI function set_state_var()
to set all state variable values into a new instance of the model.
This new instance could be running on another node.

This pair of functions could be used for the problem of
load balancing or for recovery after hardware failures.

See this documentation for msgpack:
https://github.com/msgpack/msgpack-c/wiki/v2_0_c_overview

See this msgpack example:
https://blog.gypsyengineer.com/en/security/msgpack-fuzzing.html
*/

//-----------------------------------------------------------------------
int serialize(Bmi* model1, const char *ser_file)
{
  int n_state_vars;
  model1->get_state_var_count(model1, &n_state_vars);
    
  FILE *fp = fopen(ser_file, "w+");
  int i, j;  
  void *ptr_list[ n_state_vars ];
  unsigned int size, sizes[ n_state_vars ];
  int verbose = 1;

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

      //-------------------------------------
      // Is this state var a single value ?
      //-------------------------------------
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
				  msgpack_pack_int(&pk, *( ((int *)ptr_list[i]) + j)); }
		  } else if (strcmp(type, "float") == 0){
			  for (j=0; j<size; j++){
				  msgpack_pack_float(&pk, *( ((float *)ptr_list[i]) + j)); }
		  } else if (strcmp(type, "double") == 0){
			  for (j=0; j<size; j++){
				  msgpack_pack_double(&pk, *( ((double *)ptr_list[i]) + j)); }
		  } else if (strcmp(type, "string") == 0){
			  // Note:  Need ptr_list[i] without * here.
			  msgpack_pack_str_body(&pk, ptr_list[i], size);   
// 		  } else if (strcmp(type, "FILE") == 0){
// 			  // nil is an object pointer to nothing
// 			  msgpack_pack_nil(&pk);  // Need something; will this work?
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
    int j, ival;
    void *ptr;      //###############
    float fval;
    double dval;    //##############
    
    char *type;
    char *sval;
    char **types = NULL;
    types = (char**) malloc (sizeof(char *) * n_state_vars);  
    for (int j=0; j<n_state_vars; j++){
        types[j] = (char*) malloc (sizeof(char) * BMI_MAX_VAR_NAME);
    }
    model2->get_state_var_types(model2, types);

    char inbuffer[BUFFER_SIZE]; 
    FILE *fp = fopen(ser_file, "rb");
    int i=0;
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
//             ptr = &obj;
//             model2->set_state_var(model2, ptr, i );
            //--------------------------------------------			
			if (strcmp(type, "int") == 0){
				ival = (int)obj.via.i64;
				model2->set_state_var(model2, &ival, i );
			} else if (strcmp(type, "float") == 0){        
				fval = (float)obj.via.f64;
				model2->set_state_var(model2, &fval, i );
			} else if (strcmp(type, "double") == 0){   
				dval = (double)obj.via.f64;
				model2->set_state_var(model2, &dval, i );
			} else if (strcmp(type, "string") == 0){
				// Seems to work for title & subcat
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
            //---------------------------------------------------
            // Get ptr to first value in array
            // set_state_var() will use size to set all values
            // in array and will also typecast.
            //---------------------------------------------------
            // This seems to work for var: "dbl_array_test"
            // but it is just an array, not a ptr to an array.
            //---------------------------------------------------
            ptr = obj.via.array.ptr;
			model2->set_state_var(model2, ptr, i );       
        }
        
        //---------------------------------------
        // For now, assume each obj is a scalar
        //---------------------------------------
        // model2->set_state_var(model2, ptr, i );        
 
        //---------------------------------------------------
        // Note: If all objects are arrays (even 1 element)
        //       then may need something like this.
        //---------------------------------------------------       
        // ptr = &obj[0];
        // ptr = &obj.via.array.ptr[0];
        //##### ptr = obj.via.array.ptr;  // same as prev line?
        //##### model2->set_state_var(model2, ptr, i );
        
        //for (j=0; j<sizes[i]; j++){
        //     model2->set_state_var(model2, (ptr + j), i );
        //}              
        // model2->set_state_var(model2, &obj, i );    //############
        // model2->set_state_var(model2, &(obj[0]), i );    // DOESN'T WORK
        
        if (print_obj){
            printf("Object no %d:\n", i);
            msgpack_object_print(stdout, obj);
            printf("\n");
            
            //if (i == 9){
            //    printf("obj.via.array.ptr[0].via.u64 = %llu\n", obj.via.array.ptr[0].via.u64);
            //    printf("obj.via.array.ptr[0].via.i64 = %llu\n", obj.via.array.ptr[0].via.i64);
            //    printf("obj.via.array.ptr[0].via.f64 = %f\n", obj.via.array.ptr[0].via.f64);
            //}
              
            //------------------------------
            // This just prints obj again
            //------------------------------
            // msgpack_object_print_buffer(unpacked_buffer, UNPACKED_BUFFER_SIZE, obj);
            // printf("%s\n", unpacked_buffer);        
        }       
        //-----------------------------------------------------
        i++;
        ret = msgpack_unpack_next(&unpacked, inbuffer, len, &off);
    }

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


