
#include "store.hpp"


using namespace klio;


void Store::sync_readings(klio::Sensor::Ptr sensor, klio::Store::Ptr store) {

    sensor = get_sensor(sensor->uuid());

    klio::readings_t_Ptr readings = store->get_all_readings(sensor);

    update_readings(sensor, *readings);
}
