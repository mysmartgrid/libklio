#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/txt/txt-store.hpp>

using namespace klio;

const std::string TXTStore::ENABLED = "1";
const std::string TXTStore::DISABLED = "0";
const std::string TXTStore::NOT_A_NUMBER = "nan";
const std::string TXTStore::DEFAULT_FIELD_SEPARATOR = ",";

void TXTStore::open() {
}

void TXTStore::close() {
}

void TXTStore::check_integrity() {

    const std::string path = compose_sensors_path();

    if (!bfs::exists(path)) {
        std::ostringstream oss;
        oss << "The database path is incomplete.";
        throw StoreException(oss.str());
    }

    const bfs::directory_iterator end;

    for (bfs::directory_iterator it(path); it != end; it++) {

        try {
            boost::uuids::uuid uuid;
            std::stringstream ss;
            ss << it->path().filename().string();
            ss >> uuid;

        } catch (std::exception e) {
            std::ostringstream oss;
            oss << "The database path contains invalid subdirectories.";
            throw StoreException(oss.str());
        }
    }
}

void TXTStore::initialize() {

    bfs::remove_all(_path);
    create_directory(_path.string());
    create_directory(compose_sensors_path());
}

void TXTStore::dispose() {

    close();
    bfs::remove_all(_path);
}

const std::string TXTStore::str() {

    std::ostringstream oss;
    oss << "CSV database, stored in path " << _path.string();
    return oss.str();
}

void TXTStore::add_sensor_record(const Sensor::Ptr sensor) {

    check_sensor(sensor, false);

    const std::string uuid = sensor->uuid_string();
    create_directory(compose_sensor_path(uuid));

    std::ofstream file(compose_sensor_properties_path(uuid),
            std::ofstream::out | std::ofstream::app);

    try {
        save_sensor(file, sensor);
        file.close();

    } catch (const std::exception& e) {
        file.close();
        throw StoreException(e.what());
    }
}

void TXTStore::remove_sensor_record(const Sensor::Ptr sensor) {

    bfs::remove_all(compose_sensor_path(sensor->uuid_string()));
}

void TXTStore::update_sensor_record(const Sensor::Ptr sensor) {

    check_sensor(sensor, true);

    std::ofstream file(compose_sensor_properties_path(sensor->uuid_string()),
            std::ofstream::out | std::ofstream::app);

    try {
        save_sensor(file, sensor);
        file.close();

    } catch (const std::exception& e) {
        file.close();
        throw StoreException(e.what());
    }
}

void TXTStore::add_single_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value, const bool ignore_errors) {

    check_sensor(sensor, true);

    std::ofstream file(compose_sensor_readings_path(sensor->uuid_string()),
            std::ofstream::out | std::ofstream::app);

    try {
        save_reading(file, timestamp, value);
        file.close();

    } catch (std::exception const& e) {
        file.close();
        handle_reading_insertion_error(ignore_errors, timestamp, value);
    }
}

void TXTStore::add_bulk_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    check_sensor(sensor, true);

    std::ofstream file(compose_sensor_readings_path(sensor->uuid_string()),
            std::ofstream::out | std::ofstream::app);

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

        try {
            save_reading(file, (*it).first, (*it).second);

        } catch (std::exception const& e) {
            handle_reading_insertion_error(ignore_errors, (*it).first, (*it).second);
        }
    }
    file.close();
}

void TXTStore::update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    check_sensor(sensor, true);
    
    std::ofstream file(compose_sensor_readings_path(sensor->uuid_string()),
            std::ofstream::out | std::ofstream::app);

    for (readings_cit_t it = readings.begin(); it != readings.end(); ++it) {

        try {
            save_reading(file, (*it).first, (*it).second);

        } catch (std::exception const& e) {
            handle_reading_insertion_error(ignore_errors, (*it).first, (*it).second);
        }
    }
    file.close();
}

std::vector<Sensor::Ptr> TXTStore::get_sensor_records() {

    //TODO: improve this method
    std::vector<Sensor::Ptr> sensors;
    std::vector<std::vector < std::string>> records = read_records(compose_sensors_path());

    for (std::vector<std::vector < std::string>>::iterator record = records.begin(); record != records.end(); ++record) {

        sensors.push_back(sensor_factory->createSensor(
                (*record).at(1), //uuid,
                (*record).at(2), //external_id
                (*record).at(3), //name
                (*record).at(4), //"description
                (*record).at(5), //unit
                (*record).at(6) //timezone
                ));
    }
    return sensors;
}

readings_t_Ptr TXTStore::get_all_reading_records(const Sensor::Ptr sensor) {

    check_sensor(sensor, true);
    
    //TODO: improve this method
    readings_t_Ptr readings(new readings_t());
    std::vector<std::vector < std::string>> records = read_records(compose_sensor_readings_path(sensor->uuid_string()));

    for (std::vector<std::vector < std::string>>::iterator record = records.begin(); record != records.end(); ++record) {

        std::string value = (*record).at(2);
        std::transform(value.begin(), value.end(), value.begin(), ::tolower);

        if (value.find(NOT_A_NUMBER) == std::string::npos) {

            try {
                timestamp_t timestamp = boost::lexical_cast<timestamp_t>((*record).at(1));

                readings->insert(std::pair<timestamp_t, double>(
                        time_converter->convert_from_epoch(timestamp),
                        boost::lexical_cast<double>(value)));

            } catch (boost::bad_lexical_cast &) {
                //Ignore
            }
        }
    }
    return readings;
}

readings_t_Ptr TXTStore::get_timeframe_reading_records(const Sensor::Ptr sensor, const timestamp_t begin, const timestamp_t end) {

    //TODO: improve this method
    readings_t_Ptr readings(new readings_t());
    readings_t_Ptr all = get_all_reading_records(sensor);

    for (std::map<klio::timestamp_t, double>::iterator it = all->begin(); it != all->end(); ++it) {

        klio::timestamp_t timestamp = (*it).first;

        if (timestamp >= begin && timestamp <= end) {
            readings->insert(*it);
        }
    }
    return readings;
}

unsigned long int TXTStore::get_num_readings_value(const Sensor::Ptr sensor) {

    //TODO: make this method more efficient
    return get_all_readings(sensor)->size();
}

reading_t TXTStore::get_last_reading_record(const Sensor::Ptr sensor) {

    //TODO: improve this method
    return *(get_all_reading_records(sensor)->end());
}

reading_t TXTStore::get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    //TODO: improve this method
    readings_t_Ptr all = get_all_reading_records(sensor);

    for (std::map<klio::timestamp_t, double>::iterator it = all->begin(); it != all->end(); ++it) {

        if ((*it).first == timestamp) {
            return *it;
        }
    }
    return std::pair<timestamp_t, double>(0, 0);
}

void TXTStore::save_sensor(std::ofstream& file, const Sensor::Ptr sensor) {

    file << ENABLED << _field_separator <<
            sensor->uuid() << _field_separator <<
            sensor->external_id() << _field_separator <<
            sensor->name() << _field_separator <<
            sensor->description() << _field_separator <<
            sensor->unit() << _field_separator <<
            sensor->timezone() << std::endl;
}

void TXTStore::save_reading(std::ofstream& file, const timestamp_t& timestamp, const double value) {

    file << ENABLED << _field_separator <<
            std::to_string(timestamp) << _field_separator <<
            std::to_string(value) << std::endl;
}

std::vector<std::vector<std::string>> TXTStore::read_records(const std::string& path) {

    std::vector<std::vector < std::string>> records;
    std::vector<std::string> record;

    std::map<std::string, std::vector < std::string>> lines;
    std::string line;
    std::ifstream file(path.c_str());

    try {
        while (getline(file, line)) {

            boost::tokenizer<boost::char_separator<char> > tokenizer(line, _token_separator);

            if (tokenizer.begin() != tokenizer.end()) {

                record.assign(tokenizer.begin(), tokenizer.end());

                //Status
                if (record.at(0) == ENABLED) {
                    lines.insert(std::pair<std::string, std::vector < std::string >> (record.at(1), record));

                } else {
                    lines.erase(record.at(1));
                }
            }
        }
        file.close();

    } catch (std::exception& e) {
        file.close();
        throw StoreException("Invalid CVS file format");
    }

    for (std::map<std::string, std::vector < std::string>>::iterator it = lines.begin(); it != lines.end(); ++it) {
        records.push_back(it->second);
    }
    return records;
}

void TXTStore::check_sensor(const Sensor::Ptr sensor, const bool should_exist) {

    const std::string uuid = sensor->uuid_string();
    const std::string path = compose_sensor_properties_path(uuid);

    if (bfs::exists(path)) {
        
        if (!should_exist) {
            std::ostringstream oss;
            oss << "Sensor " << path << " already exists";
            throw StoreException(oss.str());
        }

    } else if (should_exist) {
        std::ostringstream oss;
        oss << "Sensor " << uuid << " not found";
        throw StoreException(oss.str());
    }
}

const std::string TXTStore::compose_sensors_path() {

    std::ostringstream str;
    str << _path.string() << "/sensors";
    return str.str();
}

const std::string TXTStore::compose_sensor_path(const std::string& uuid) {

    std::ostringstream str;
    str << compose_sensors_path() << "/" << uuid;
    return str.str();
}

const std::string TXTStore::compose_sensor_properties_path(const std::string& uuid) {

    std::ostringstream str;
    str << compose_sensor_path(uuid) << "/properties.csv";
    return str.str();
}

const std::string TXTStore::compose_sensor_readings_path(const std::string& uuid) {

    std::ostringstream str;
    str << compose_sensor_path(uuid) << "/readings.csv";
    return str.str();
}

void TXTStore::create_directory(const std::string& dir) {

    if (!bfs::create_directory(dir)) {

        std::ostringstream str;
        str << "CSV database directory " << dir << " can not be created.";
        throw StoreException(str.str());
    }
}