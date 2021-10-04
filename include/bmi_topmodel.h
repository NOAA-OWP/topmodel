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
    char name[80];
    char type[80];
    unsigned int size;
    // char units[80];
    // char role[80];
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
