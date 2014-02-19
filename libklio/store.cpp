
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

    std::vector<klio::Sensor::Ptr> sensors = get_sensors_by_external_id(sensor->external_id());

    if (sensors.empty()) {

        add_sensor(sensor);

    } else {

        for (std::vector<klio::Sensor::Ptr>::const_iterator it = sensors.begin(); it != sensors.end(); ++it) {

            klio::Sensor::Ptr found = (*it);
            
            if (found->name() != sensor->name() ||
                    found->description() != sensor->description() ||
                    found->unit() != sensor->unit() ||
                    found->timezone() != sensor->timezone()) {

                update_sensor(sensor);
            }
        }
    }

    klio::readings_t_Ptr readings = store->get_all_readings(sensor);

    update_readings(sensor, *readings);
}