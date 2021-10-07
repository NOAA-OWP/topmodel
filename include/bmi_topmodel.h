#ifndef BMI_TOPMODEL_H
#define BMI_TOPMODEL_H

#if defined(__cplusplus)
extern "C" {
#endif

#include "bmi.h"
#include "topmodel.h"

//--------------------------------------------
// Used to simplify BMI implementation (SDP)
//--------------------------------------------
typedef struct Variable{
    unsigned int index;
    char name[BMI_MAX_VAR_NAME]; 
    char type[BMI_MAX_TYPE_NAME];
    unsigned int size;
    char role[BMI_MAX_ROLE_NAME];
    char units[BMI_MAX_UNITS_NAME];
    unsigned int grid;
    char location[BMI_MAX_LOCATION_NAME];
    // bool is_pointer;
} Variable;

Bmi* register_bmi_topmodel(Bmi *model);

topmodel_model * new_bmi_topmodel();

int init_config(const char* config_file, topmodel_model* model);

int read_init_config(const char* config_file, topmodel_model* model);


#if defined(__cplusplus)
}
#endif

#endif
