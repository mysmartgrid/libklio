
#include "store.hpp"


using namespace klio;

reading_t Store::get_reading(klio::Sensor::Ptr sensor, timestamp_t timestamp) {

    //This default implementation should be overridden by concrete classes.

    klio::readings_t_Ptr readings = get_all_readings(sensor);

    klio::readings_cit_t it;
    for (it = readings->begin(); it != readings->end(); ++it) {

        if ((*it).first == timestamp) {
            return (*it);
        }
    }
    return std::pair<timestamp_t, double>(0, 0);
}

void Store::sync(klio::Store::Ptr store) {

    std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();

    for (std::vector<klio::Sensor::uuid_t>::const_iterator uuid = uuids.begin(); uuid != uuids.end(); ++uuid) {

        sync_readings(store->get_sensor(*uuid), store);
    }
}

void Store::sync_readings(klio::Sensor::Ptr sensor, klio::Store::Ptr store) {

    try {
        klio::Sensor::Ptr retrieved = get_sensor(sensor->uuid());

        if (retrieved->external_id() != sensor->external_id() ||
                retrieved->name() != sensor->name() ||
                retrieved->description() != sensor->description() ||
                retrieved->unit() != sensor->unit() ||
                retrieved->timezone() != sensor->timezone()) {

            update_sensor(sensor);
        }

    } catch (klio::StoreException const& e) {

        //TODO: no exception should be raised when a sensor is not found
        
        add_sensor(sensor);
    }

    klio::readings_t_Ptr readings = store->get_all_readings(sensor);

    update_readings(sensor, *readings);
}
