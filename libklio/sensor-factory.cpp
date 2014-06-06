#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/local-time.hpp>
#include <libklio/error.hpp>
#include "sensor-factory.hpp"


using namespace klio;

klio::Sensor::Ptr SensorFactory::createSensor(
        const std::string& external_id,
        const std::string& name,
        const std::string& unit,
        const std::string& timezone
        ) {
    return createSensor(external_id, name, unit, timezone, DeviceType::UNKNOWN_DEVICE);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const std::string& external_id,
        const std::string& name,
        const std::string& unit,
        const std::string& timezone,
        const klio::DeviceType::Ptr device_type
        ) {
    boost::uuids::random_generator gen_uuid;
    return createSensor(external_id, name, klio::DEFAULT_SENSOR_DESCRIPTION, unit, timezone, device_type);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const std::string& external_id,
        const std::string& name,
        const std::string& description,
        const std::string& unit,
        const std::string& timezone,
        const klio::DeviceType::Ptr device_type
        ) {
    boost::uuids::random_generator gen_uuid;
    return createSensor(gen_uuid(), external_id, name, description, unit, timezone, device_type);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const std::string& uuid_string,
        const std::string& external_id,
        const std::string& name,
        const std::string& unit,
        const std::string& timezone
        ) {
    return createSensor(uuid_string, external_id, name, klio::DEFAULT_SENSOR_DESCRIPTION, unit, timezone);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const std::string& uuid_string,
        const std::string& external_id,
        const std::string& name,
        const std::string& description,
        const std::string& unit,
        const std::string& timezone
        ) {

    return createSensor(uuid_string, external_id, name, description, unit, timezone, DeviceType::UNKNOWN_DEVICE);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const std::string& uuid_string,
        const std::string& external_id,
        const std::string& name,
        const std::string& description,
        const std::string& unit,
        const std::string& timezone,
        const klio::DeviceType::Ptr device_type
        ) {

    // type conversion: uuid_string to real uuid type
    boost::uuids::uuid u;
    std::stringstream ss;
    ss << uuid_string;
    ss >> u;

    return createSensor(u, external_id, name, description, unit, timezone, device_type);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const Sensor::uuid_t& uuid,
        const std::string& external_id,
        const std::string& name,
        const std::string& description,
        const std::string& unit,
        const std::string& timezone
        ) {

    return createSensor(uuid, external_id, name, description, unit, timezone, DeviceType::UNKNOWN_DEVICE);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const Sensor::uuid_t& uuid,
        const std::string& external_id,
        const std::string& name,
        const std::string& description,
        const std::string& unit,
        const std::string& timezone,
        const klio::DeviceType::Ptr device_type
        ) {
    
    klio::LocalTime::Ptr local_time(new klio::LocalTime("../.."));

    if (!local_time->is_valid_timezone(timezone)) {

        std::vector<std::string> valid_regions(local_time->get_valid_timezones());

        std::ostringstream oss;
        oss << "Invalid timezone " << timezone << ". Valid timezones are: " << std::endl;
        std::copy(valid_regions.begin(), valid_regions.end(), std::ostream_iterator<std::string > (oss, ", "));

        throw klio::DataFormatException(oss.str());
    }
    
    return Sensor::Ptr(new Sensor(uuid, external_id, name, description, unit, timezone, device_type));
}