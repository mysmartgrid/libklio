#include <iostream>
#include <iterator>
#include <boost/multi_array.hpp>
#include <boost/array.hpp>
#include <algorithm>
#include <libklio/algorithm/collate.hpp>

std::vector<double> klio::get_sensordata_row(
        const klio::sensordata_table_t& table, unsigned long int row_index) {
    std::vector<double> retval;
    klio::sensordata_array_t data = table.get < 0 > ();
    if (row_index <= table.get < 1 > ()) {
        for (size_t columnidx = 0; columnidx < table.get < 2 > (); columnidx++) {
            retval.push_back(data[row_index][columnidx]);
        }
    } else {
        throw klio::DataFormatException("cannot collect sensordata: request for invalid row index.");
    }
    return retval;
}

klio::sensordata_table_t
klio::collate(Store::Ptr store, const sensors_t& sensors) {
    // 1. Check if all sensors have the same size.
    unsigned long int timestampcount = 0;
    bool first_sensor = true;
    for (klio::sensors_cit_t it = sensors.begin(); it < sensors.end(); ++it) {
        klio::Sensor::Ptr current = (*it);
        if (first_sensor) {
            timestampcount = store->get_num_readings(current);
            first_sensor = false;
        } else {
            if (store->get_num_readings(current) != timestampcount)
                throw klio::DataFormatException("cannot collate: sensors have different number of datapoints");
        }
    }
    // 2. Create return variable based on the determined size, transfer data
    klio::sensordata_array_t retval(boost::extents[timestampcount][1 + sensors.size()]);
    size_t sensor_idx = 1;
    for (klio::sensors_cit_t it = sensors.begin(); it < sensors.end(); ++it) {
        readings_t_Ptr readings = store->get_all_readings((*it));
        size_t timestamp_idx = 0;
        for (readings_it_t it = readings->begin(); it != readings->end(); it++) {
            if (sensor_idx == 1) { // first sensor, copy the timestamps
                //retval[0][timestamp_idx] = (double)(*it).first;
                //retval[sensor_idx][timestamp_idx] = (*it).second;
                retval[timestamp_idx][0] = (double) (*it).first;
                retval[timestamp_idx][sensor_idx] = (*it).second;
            } else { // all other sensors: compare the timestamps
                //if (retval[0][timestamp_idx] == (double)(*it).first) {
                //  retval[sensor_idx][timestamp_idx] = (*it).second;
                if (retval[timestamp_idx][0] == (double) (*it).first) {
                    retval[timestamp_idx][sensor_idx] = (*it).second;
                } else {
                    throw klio::DataFormatException("cannot collate: timestamps of sensors do not match.");
                }
            }
            // upcoming: the next timestamp.
            ++timestamp_idx;
        }
        // now: Increase the current sensor index
        ++sensor_idx;
    }

    //FIXME
    //return make_tuple(retval, (unsigned long int) timestampcount, (unsigned long int) (1 + sensors.size()));
    klio::sensordata_table_t x;
    return x;
}
