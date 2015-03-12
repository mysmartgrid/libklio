#include <libklio/config.h>

#ifdef ENABLE_MSG

#include <jsoncpp/json/json.h>
#include <libmysmartgrid/error.h>
#include <libklio/msg/msg-store.hpp>


using namespace klio;

const std::string MSGStore::DEFAULT_MSG_URL = "https://api.mysmartgrid.de:8443";
const std::string MSGStore::DEFAULT_MSG_DESCRIPTION = "libklio mSG Store";
const std::string MSGStore::DEFAULT_MSG_TYPE = "libklio";

void MSGStore::open() {
    //Nothing is done with the remote store
}

void MSGStore::close() {
    Store::clear_buffers();
}

void MSGStore::check_integrity() {
    heartbeat();
}

void MSGStore::initialize() {

    try {
        libmsg::JsonPtr jobject(new Json::Value(Json::objectValue));
        (*jobject)["key"] = _key;
        (*jobject)["description"] = _description;
        (*jobject)["type"] = _type;

        std::string url = libmsg::Webclient::composeDeviceUrl(_url, _id);
        libmsg::Webclient::performHttpPost(url, libmsg::Secret::fromKey(_key), jobject);

    } catch (libmsg::MemoryException const & e) {
        throw MemoryException(e.what());

    } catch (libmsg::EnvironmentException const & e) {
        throw EnvironmentException(e.what());

    } catch (libmsg::DataFormatException const & e) {
        throw DataFormatException(e.what());

    } catch (libmsg::CommunicationException const & e) {
        throw CommunicationException(e.what());

    } catch (libmsg::GenericException const & e) {
        throw GenericException(e.what());

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

void MSGStore::prepare() {

    try {
        std::string url = libmsg::Webclient::composeDeviceUrl(_url, _id);
        libmsg::JsonPtr jobject = libmsg::Webclient::performHttpGet(url, libmsg::Secret::fromKey(_key));

        _description = (*jobject)["description"].asString();
        _type = (*jobject)["type"].asString();

        Json::Value jsensors = (*jobject)["sensors"];
        for (Json::Value::iterator it = jsensors.begin(), end = jsensors.end(); it != end; it++) {
            std::string meter = (*it)["meter"].asString();
            set_buffers(parse_sensor(
                    format_uuid_string(meter),
                    *it));
        }

    } catch (libmsg::MemoryException const & e) {
        throw MemoryException(e.what());

    } catch (libmsg::EnvironmentException const & e) {
        throw EnvironmentException(e.what());

    } catch (libmsg::DataFormatException const & e) {
        throw DataFormatException(e.what());

    } catch (libmsg::CommunicationException const & e) {
        throw CommunicationException(e.what());

    } catch (libmsg::GenericException const & e) {
        throw GenericException(e.what());

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

void MSGStore::dispose() {

    //Tries to delete the remote store
    try {
        std::string url = libmsg::Webclient::composeDeviceUrl(_url, _id);
        libmsg::Webclient::performHttpDelete(url, libmsg::Secret::fromKey(_key));

    } catch (libmsg::GenericException const& e) {
        //Ignore failed attempt

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
    close();
}

void MSGStore::flush() {
    heartbeat();
    Store::flush();
}

const std::string MSGStore::str() {

    std::ostringstream oss;
    oss << "mSG store " << libmsg::Webclient::composeDeviceUrl(_url, _id);
    return oss.str();
}

void MSGStore::add_sensor_record(const Sensor::Ptr sensor) {

    update_sensor_record(sensor);
}

void MSGStore::remove_sensor_record(const Sensor::Ptr sensor) {

    try {
        std::string url = libmsg::Webclient::composeSensorUrl(_url, sensor->uuid_short());
        libmsg::Webclient::performHttpDelete(url, libmsg::Secret::fromKey(_key));

    } catch (libmsg::MemoryException const & e) {
        throw MemoryException(e.what());

    } catch (libmsg::EnvironmentException const & e) {
        throw EnvironmentException(e.what());

    } catch (libmsg::DataFormatException const & e) {
        throw DataFormatException(e.what());

    } catch (libmsg::CommunicationException const & e) {
        throw CommunicationException(e.what());

    } catch (libmsg::GenericException const & e) {
        throw GenericException(e.what());

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

void MSGStore::update_sensor_record(const Sensor::Ptr sensor) {

    try {
        Json::Value jconfig(Json::objectValue);
        jconfig["device"] = _id;
        jconfig["externalid"] = sensor->external_id();
        jconfig["function"] = sensor->name();
        jconfig["description"] = sensor->description();
        jconfig["unit"] = sensor->unit();

        libmsg::JsonPtr jobject(new Json::Value(Json::objectValue));
        (*jobject)["config"] = jconfig;

        std::string url = libmsg::Webclient::composeSensorUrl(_url, sensor->uuid_short());
        libmsg::Webclient::performHttpPost(url, libmsg::Secret::fromKey(_key), jobject);

    } catch (libmsg::MemoryException const & e) {
        throw MemoryException(e.what());

    } catch (libmsg::EnvironmentException const & e) {
        throw EnvironmentException(e.what());

    } catch (libmsg::DataFormatException const & e) {
        throw DataFormatException(e.what());

    } catch (libmsg::CommunicationException const & e) {
        throw CommunicationException(e.what());

    } catch (libmsg::GenericException const & e) {
        throw GenericException(e.what());

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

void MSGStore::add_single_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp, const double value, const bool ignore_errors) {

    try {
        Json::Value jmeasurements(Json::arrayValue);

        Json::Value jtuple(Json::arrayValue);
        jtuple.append(Json::Int64(timestamp));
        jtuple.append(value);

        jmeasurements.append(jtuple);

        libmsg::JsonPtr jobject(new Json::Value(Json::objectValue));
        (*jobject)["measurements"] = jmeasurements;

        std::string url = libmsg::Webclient::composeSensorUrl(_url, sensor->uuid_short());
        libmsg::Webclient::performHttpPost(url, libmsg::Secret::fromKey(_key), jobject);

    } catch (libmsg::GenericException const& e) {
        handle_reading_insertion_error(ignore_errors, sensor);

    } catch (std::exception const& e) {
        handle_reading_insertion_error(ignore_errors, sensor);
    }
}

void MSGStore::add_bulk_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    try {
        Json::Value jmeasurements(Json::arrayValue);

        for (readings_cit_t rit = readings.begin(); rit != readings.end(); ++rit) {

            timestamp_t timestamp = (*rit).first;
            double value = (*rit).second;

            Json::Value jtuple(Json::arrayValue);
            jtuple.append(Json::Int64(timestamp));
            jtuple.append(value);

            jmeasurements.append(jtuple);
        }

        libmsg::JsonPtr jobject(new Json::Value(Json::objectValue));
        (*jobject)["measurements"] = jmeasurements;

        std::string url = libmsg::Webclient::composeSensorUrl(_url, sensor->uuid_short());
        libmsg::Webclient::performHttpPost(url, libmsg::Secret::fromKey(_key), jobject);

    } catch (libmsg::GenericException const& e) {
        handle_reading_insertion_error(ignore_errors, sensor);

    } catch (std::exception const& e) {
        handle_reading_insertion_error(ignore_errors, sensor);
    }
}

void MSGStore::update_reading_records(const Sensor::Ptr sensor, const readings_t& readings, const bool ignore_errors) {

    add_bulk_reading_records(sensor, readings, ignore_errors);
}

std::vector<Sensor::Ptr> MSGStore::get_sensor_records() {

    try {
        std::string url = libmsg::Webclient::composeDeviceUrl(_url, _id);
        libmsg::JsonPtr jobject = libmsg::Webclient::performHttpGet(url, libmsg::Secret::fromKey(_key));

        std::vector<Sensor::Ptr> sensors;
        Json::Value jsensors = (*jobject)["sensors"];

        for (Json::Value::iterator it = jsensors.begin(), end = jsensors.end(); it != end; it++) {
            std::string meter = (*it)["meter"].asString();
            sensors.push_back(parse_sensor(
                    format_uuid_string(meter),
                    *it));
        }
        return sensors;

    } catch (libmsg::MemoryException const & e) {
        throw MemoryException(e.what());

    } catch (libmsg::EnvironmentException const & e) {
        throw EnvironmentException(e.what());

    } catch (libmsg::DataFormatException const & e) {
        throw DataFormatException(e.what());

    } catch (libmsg::CommunicationException const & e) {
        throw CommunicationException(e.what());

    } catch (libmsg::GenericException const & e) {
        throw GenericException(e.what());

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

readings_t_Ptr MSGStore::get_all_reading_records(const Sensor::Ptr sensor) {

    try {
        libmsg::Webclient::ReadingList l = libmsg::Webclient::getReadings(_url, sensor->uuid_short(), libmsg::Secret::fromKey(_key), sensor->unit());

        readings_t_Ptr readings(new readings_t(l.begin(), l.end()));

        return readings;

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

readings_t_Ptr MSGStore::get_timeframe_reading_records(const Sensor::Ptr sensor, timestamp_t begin, timestamp_t end) {

    //TODO: improve this method

    readings_t_Ptr selected;
    readings_t_Ptr readings = get_all_readings(sensor);

    for (readings_cit_t reading = readings->begin(); reading != readings->end(); ++reading) {

        if ((*reading).first >= begin && (*reading).first <= end) {
            selected->insert(*reading);
        }
    }
    return selected;
}

unsigned long int MSGStore::get_num_readings_value(const Sensor::Ptr sensor) {

    try {
        libmsg::Webclient::ReadingList l = libmsg::Webclient::getReadings(_url, sensor->uuid_short(), libmsg::Secret::fromKey(_key), sensor->unit());
        long int num = l.size();

        return num;

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

reading_t MSGStore::get_last_reading_record(const Sensor::Ptr sensor) {

    try {
        libmsg::Webclient::Reading reading = libmsg::Webclient::getLastReading(_url, sensor->uuid_short(), libmsg::Secret::fromKey(_key), sensor->unit());

        return reading;

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

reading_t MSGStore::get_reading_record(const Sensor::Ptr sensor, const timestamp_t timestamp) {

    //FIXME: make this method more efficient
    readings_t_Ptr readings = get_all_readings(sensor);

    std::pair<timestamp_t, double> reading = std::pair<timestamp_t, double>(0, 0);

    if (readings->count(timestamp)) {
        reading.first = timestamp;
        reading.second = readings->at(timestamp);
    }
    return reading;
}

void MSGStore::heartbeat() {

    try {
        timestamp_t now = time_converter->get_timestamp();

        //Send a heartbeat every 30 minutes
        if (now - _last_heartbeat > 1800) {

            std::string url = libmsg::Webclient::composeDeviceUrl(_url, _id);
            libmsg::Webclient::performHttpPost(url, libmsg::Secret::fromKey(_key), libmsg::JsonPtr(new Json::Value()));

            _last_heartbeat = now;
        }

    } catch (libmsg::MemoryException const & e) {
        throw MemoryException(e.what());

    } catch (libmsg::EnvironmentException const & e) {
        throw EnvironmentException(e.what());

    } catch (libmsg::DataFormatException const & e) {
        throw DataFormatException(e.what());

    } catch (libmsg::CommunicationException const & e) {
        throw CommunicationException(e.what());

    } catch (libmsg::GenericException const & e) {
        throw GenericException(e.what());

    } catch (std::exception const& e) {
        throw EnvironmentException(e.what());
    }
}

const std::string MSGStore::format_uuid_string(const std::string & meter) {

    std::stringstream oss;
    for (size_t i = 0; i < meter.length(); i++) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            oss << '-';
        }
        oss << meter[i];
    }
    return oss.str();
}

Sensor::Ptr MSGStore::parse_sensor(const std::string& uuid_str, const Json::Value& jsensor) {

    Json::Value jname = jsensor["function"];
    Json::Value jdescription = jsensor["description"];
    Json::Value jexternal_id = jsensor["externalid"];
    Json::Value junit = jsensor["unit"];

    //TODO: there should be no hard coded default timezone
    std::string timezone = "Europe/Berlin";

    return sensor_factory->createSensor(
            uuid_str,
            jexternal_id.asString(),
            jname.asString(),
            jdescription.asString(),
            junit.asString(),
            timezone);
}

std::pair<timestamp_t, double> MSGStore::create_reading_pair(const Json::Value& jpair) {

    if (jpair[1].isConvertibleTo(Json::realValue)) {
        const double value = jpair[1].asDouble();

        if (!isnan(value)) {
            Json::Int64 jtimestamp = jpair[0].asInt64();
            timestamp_t timestamp = time_converter->convert_from_epoch(jtimestamp);

            return std::pair<timestamp_t, double>(timestamp, value);
        }
    }

    return std::pair<timestamp_t, double>(0, 0);
}

#endif /* ENABLE_MSG */
