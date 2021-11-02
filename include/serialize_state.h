
#ifndef SERIALIZE_STATE_H
#define SERIALIZE_STATE_H

#if defined(__cplusplus)
extern "C" {
#endif

int get_file_size(const char *ser_file, unsigned long int *file_size);

int get_state_var_count(Bmi *model1, int *count);

int get_state_var_names(Bmi *model1, char **names);

int get_state_var_types(Bmi *model1, char **names, char **types);

int get_state_var_sizes(Bmi *model1, char **names, unsigned int sizes[]);

int get_state_var_ptrs(Bmi *model1, char **names, void *ptrs[]);

//------------------------------------------------------------------------
int serialize(Bmi* model1, const char *ser_file);

int deserialize_to_state(const char *ser_file, Bmi* model2, int print_obj);

int compare_states( Bmi* model1, Bmi* model2);

#if defined(__cplusplus)
}
#endif

#endif