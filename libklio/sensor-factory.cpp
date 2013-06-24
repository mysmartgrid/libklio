#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <libklio/local-time.hpp>
#include <libklio/error.hpp>
#include "sensor-factory.hpp"


using namespace klio;


klio::Sensor::Ptr SensorFactory::createSensor(
        const std::string& name,
        const std::string& unit,
        const std::string& timezone
        ) {
    boost::uuids::uuid u = _gen();
    return createSensor(u, name, klio::DEFAULT_SENSOR_DESCRIPTION, unit, timezone);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const std::string& uuid_string,
        const std::string& name,
        const std::string& unit,
        const std::string& timezone
        ) {
    return createSensor(uuid_string, name, klio::DEFAULT_SENSOR_DESCRIPTION, unit, timezone);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const std::string& uuid_string,
        const std::string& name,
        const std::string& description,
        const std::string& unit,
        const std::string& timezone
        ) {
    
    // type conversion: uuid_string to real uuid type
    boost::uuids::uuid u;
    std::stringstream ss;
    ss << uuid_string;
    ss >> u;

    return createSensor(u, name, description, unit, timezone);
}

klio::Sensor::Ptr SensorFactory::createSensor(
        const Sensor::uuid_t& uuid,
        const std::string& name,
        const std::string& description,
        const std::string& unit,
        const std::string& timezone
        ) {
    klio::LocalTime::Ptr lt(new klio::LocalTime("../.."));

    if (!lt->is_valid_timezone(timezone)) {

        std::vector<std::string> valid_regions(lt->get_valid_timezones());

        std::ostringstream oss;
        oss << "Invalid timezone " << timezone << ". Valid timezones are: " << std::endl;
        std::copy(valid_regions.begin(), valid_regions.end(), std::ostream_iterator<std::string > (oss, ", "));

        throw klio::DataFormatException(oss.str());
    }
    return Sensor::Ptr(new Sensor(uuid, name, description, unit, timezone));
}
