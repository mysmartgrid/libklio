
#include <libklio/sensor-factory.hpp>
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

    const std::vector<klio::Sensor::Ptr> sensors = store->get_sensors();

    for (std::vector<klio::Sensor::Ptr>::const_iterator sensor = sensors.begin(); sensor != sensors.end(); ++sensor) {

        sync_readings(*sensor, store);
    }
}

void Store::sync_readings(klio::Sensor::Ptr sensor, klio::Store::Ptr store) {

    klio::readings_t_Ptr readings = store->get_all_readings(sensor);
    std::vector<klio::Sensor::Ptr> sensors = get_sensors_by_external_id(sensor->external_id());
    klio::Sensor::Ptr local_sensor;

    if (sensors.empty()) {

        add_sensor(sensor);
        local_sensor = sensor;

    } else {
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        //Exactly one sensor is processed
        for (std::vector<klio::Sensor::Ptr>::const_iterator found = sensors.begin(); found != sensors.end(); ++found) {

            local_sensor = sensor_factory->createSensor(
                    (*found)->uuid(),
                    sensor->external_id(),
                    sensor->name(),
                    sensor->description(),
                    sensor->unit(),
                    sensor->timezone(),
                    sensor->device_type());

            //Update sensor if any of its properties changed
            if ((*found)->name() != sensor->name() ||
                    (*found)->description() != sensor->description() ||
                    (*found)->unit() != sensor->unit() ||
                    (*found)->timezone() != sensor->timezone() ||
                    (*found)->device_type() != sensor->device_type()) {

                update_sensor(local_sensor);
            }
        }
    }

    update_readings(local_sensor, *readings);
}
