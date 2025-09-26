extern "C" {
#include "../include/topmodel.h"
#include "../include/logger.h"
}
#include "../include/bmi_serialization.h"

#include <fstream>
#include <streambuf>
#include <sstream>

#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

#include "../include/vecbuf.hpp"


class TopmodelSerializer {
    public:
        TopmodelSerializer(Bmi* bmi)
            : model((topmodel_model*)bmi->data) {};
        ~TopmodelSerializer() = default;

    private:
        friend class boost::serialization::access;
        topmodel_model* model;
        template<class Archive>
        void serialize(Archive& ar, const unsigned int version);
};


template<class Archive>
void TopmodelSerializer::serialize(Archive& ar, const unsigned int version) {
    topmodel_model* model = this->model;
    ar & model->current_time_step;

    // data summed between runs
    ar & model->sump; //
    ar & model->sumae; //
    ar & model->sumq; //
    ar & model->sumrz; // reassigned each update; not used for calcs
    ar & model->sumuz; // reassigned each update; not used for calcs

    // outputs of Update
    ar & model->Qout; // reassigned each update; not used for calcs
    ar & model->quz; //
    ar & model->qb; // reassigned each update; not used for calcs
    ar & model->qof; //
    ar & model->p; // reassigned each update; used in calc after assignment
    ar & model->ep; // reassigned each update; used in calc after assignment
    ar & model->sbar; // used then reassigned

    // array data that updates in update; counts set in config
    // these have an actual size 1 larger than the number
    int num_topodex_values = model->num_topodex_values + 1;
    ar & boost::serialization::make_array(
        model->deficit_root_zone, num_topodex_values
    );
    ar & boost::serialization::make_array(
        model->stor_unsat_zone, num_topodex_values
    );
    ar & boost::serialization::make_array(
        model->deficit_local, num_topodex_values
    );
    ar & boost::serialization::make_array(
        model->contrib_area, num_topodex_values
    );
    ar & boost::serialization::make_array(
        model->Q, model->num_time_delay_histo_ords + 1
    );
}


extern "C" {

/**
 * Serializes a Topmodel BMI model through boost. Formats the data as binary output for smaller memory impact than text.
 * It is the responsibility of the caller to free the newly allocated memory if BMI_SUCCESS is returned.
 * 
 * @param bmi topmodel BMI model that will be serialized
 * @param buffer Pointer to a char pointer. The pointer's pointer will be assigned to the serialized data.
 * @param size_written Pointer to the amount of data that was written to the buffer.
 * @return int signifiying whether the serialization process completed successfully.
 */
const int serialize_topmodel(Bmi* bmi) {
    TopmodelSerializer serializer(bmi);
    vecbuf<char> stream;
    boost::archive::binary_oarchive archive(stream);
    try {
        archive << serializer;
    } catch (const std::exception& e) {
        Log(LogLevel::SEVERE, "Serializing Topmodel encountered an error: %s", e.what());
        return BMI_FAILURE;
    }
    // copy serialized data into topmodel data
    topmodel_model* model = (topmodel_model*)bmi->data;
    // clear previous data if it exists
    if (model->serialized != NULL) {
        free(model->serialized);
    }
    // set size and allocate memory
    model->serialized_length = stream.size();
    model->serialized = (char*)malloc(sizeof(char) * model->serialized_length);
    // make sure memory could be allocated
    if (model->serialized == NULL) {
        model->serialized_length = 0;
        return BMI_FAILURE;
    }
    // copy stream data to new allocation
    memcpy(model->serialized, stream.data(), model->serialized_length);
    return BMI_SUCCESS;
}

 /**
  * Deserializes data into a Topmodel BMI model.
  * 
  * @param bmi Topmodel BMI model that will have values inserted into it.
  * @param buffer Start of data that wil be read as previously serialized state
  * @return int signifiying whether the serialization process completed successfully.
  */
const int deserialize_topmodel(Bmi* bmi, const char* buffer) {
    TopmodelSerializer serializer(bmi);
    std::istringstream stream(buffer);
    boost::archive::binary_iarchive archive(stream);
    try {
        archive >> serializer;
        return BMI_SUCCESS;
    } catch (const std::exception &e) {
        Log(LogLevel::SEVERE, "Deserializing Topmodel encountered an error: %s", e.what());
        return BMI_FAILURE;
    }
}

}
