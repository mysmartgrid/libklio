
#include "store.hpp"


using namespace klio;

/**
 * Returns the reading of a specific timestamp.
 * This default implementation should be overridden by concrete classes.
 * 
 * @param sensor    The sensor to be queried.
 * @param timestamp The specific timestamp.
 * @return the reading at the specific timestamp.
 */
reading_t Store::get_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp) {

    klio::readings_t_Ptr readings = get_all_readings(sensor);

    klio::readings_cit_t it;
    for (it = readings->begin(); it != readings->end(); ++it) {

        if ((*it).first == timestamp) {
            return (*it);
        }
    }

    return std::pair<timestamp_t, double>(0, 0);
}

void Store::sync_readings(klio::Sensor::Ptr sensor, klio::Store::Ptr store) {

    sensor = get_sensor(sensor->uuid());

    klio::readings_t_Ptr readings = store->get_all_readings(sensor);

    update_readings(sensor, *readings);
}
