#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string/erase.hpp>
#include <libklio/sensor.hpp>


using namespace klio;
using namespace boost::uuids;

const std::string Sensor::uuid_string() const {
    return to_string(_uuid);
}

const std::string Sensor::str() {
    std::ostringstream oss;
    oss << _name << "(" << to_string(_uuid) << "), unit: "
            << _unit << ", tz: " << _timezone << ", description: " << _description
            << ", device type: " << _device_type->name();
    return oss.str();
}

const std::string Sensor::uuid_short() const {

    return boost::algorithm::erase_all_copy(uuid_string(), "-");
}

bool Sensor::operator ==(const Sensor& rhs) {
    return _uuid == rhs.uuid() && _name == rhs.name() && _unit == rhs.unit() && _timezone == rhs.timezone() && _device_type == rhs.device_type();
}

bool Sensor::operator !=(const Sensor& rhs) {
    return not operator==(rhs);
}
