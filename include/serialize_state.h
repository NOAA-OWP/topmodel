
#ifndef SERIALIZE_STATE_H
#define SERIALIZE_STATE_H

#if defined(__cplusplus)
extern "C" {
#endif

int serialize(Bmi* model1, const char *ser_file);

int deserialize_to_state(const char *ser_file, Bmi* model2, int print_obj);

int compare_states( Bmi* model1, Bmi* model2);

#if defined(__cplusplus)
}
#endif

#endif