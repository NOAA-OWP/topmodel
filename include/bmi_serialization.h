#ifndef TOPMODEL_SERIALIZE_HPP
#define TOPMODEL_SERIALIZE_HPP

#ifdef __cplusplus
extern "C" {
#endif

#include "bmi.h"

const int serialize_topmodel(Bmi* bmi);
const int deserialize_topmodel(Bmi* bmi, const char* buffer);

#ifdef __cplusplus
}
#endif

#endif
