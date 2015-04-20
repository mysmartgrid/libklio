#include <sstream>
#include <boost/assign.hpp>
#include <libklio/device-type.hpp>


using namespace klio;

std::map<int, DeviceType::Ptr> DeviceType::_all;

const DeviceType::Ptr DeviceType::UNKNOWN_DEVICE = add_type(000, "Unknown Device");
const DeviceType::Ptr DeviceType::DISABLED_DEVICE = add_type(001, "Disabled Device");

const DeviceType::Ptr DeviceType::SINGLE_PHASE_ENERGY_CONSUMPTION = add_type(100, "Single-phase Energy Consumption");
const DeviceType::Ptr DeviceType::TRIPLE_PHASE_ENERGY_CONSUMPTION = add_type(101, "Triple-phase Energy Consumption");
const DeviceType::Ptr DeviceType::SINGLE_PHASE_ENERGY_PRODUCTION = add_type(102, "Single-phase Energy Production");
const DeviceType::Ptr DeviceType::TRIPLE_PHASE_ENERGY_PRODUCTION = add_type(103, "Triple-phase Energy Production");

const DeviceType::Ptr DeviceType::HEATING_CIRCULATING_PUMP = add_type(200, "Heating Circulating Pump");
const DeviceType::Ptr DeviceType::HEATING_PUMP = add_type(201, "Heating Pump");
const DeviceType::Ptr DeviceType::INFRARED_RADIATOR = add_type(202, "Infrared Radiator");
const DeviceType::Ptr DeviceType::SINGLE_PHASE_RADIATOR = add_type(203, "Single-phase Radiator");
const DeviceType::Ptr DeviceType::TRIPLE_PHASE_RADIATOR = add_type(204, "Triple-phase Radiator");
const DeviceType::Ptr DeviceType::AIR_CONDITIONER = add_type(205, "Air Conditioner");

const DeviceType::Ptr DeviceType::FRIDGE = add_type(300, "Fridge");
const DeviceType::Ptr DeviceType::FREEZER = add_type(301, "Freezer");
const DeviceType::Ptr DeviceType::FRIDGE_FREEZER_COMBINATION = add_type(302, "Fridge-freezer Combination");
const DeviceType::Ptr DeviceType::TOP_LID_FREEZER = add_type(303, "Top-lid Freezer");
const DeviceType::Ptr DeviceType::WASHMACHINE = add_type(304, "Wash Machine");
const DeviceType::Ptr DeviceType::DRIER = add_type(305, "Drier");
const DeviceType::Ptr DeviceType::DISHWASHER = add_type(306, "Dishwasher");
const DeviceType::Ptr DeviceType::EXPRESSO_MACHINE = add_type(307, "Expresso Machine");
const DeviceType::Ptr DeviceType::COFFEE_MACHINE = add_type(308, "Coffee Machine");
const DeviceType::Ptr DeviceType::TOASTER = add_type(309, "Toaster");
const DeviceType::Ptr DeviceType::KETTLE = add_type(310, "Kettle");
const DeviceType::Ptr DeviceType::MICROWAVE = add_type(311, "Microwave");
const DeviceType::Ptr DeviceType::HAIR_DRIER = add_type(312, "Hair Drier");
const DeviceType::Ptr DeviceType::AQUARIUM = add_type(313, "Aquarium");
const DeviceType::Ptr DeviceType::KITCHEN_AID = add_type(314, "Kitchen Aid");

const DeviceType::Ptr DeviceType::TV = add_type(400, "TV");
const DeviceType::Ptr DeviceType::RADIO = add_type(401, "Radio");
const DeviceType::Ptr DeviceType::LAMP = add_type(403, "Lamp");
const DeviceType::Ptr DeviceType::COMPUTER = add_type(404, "Computer");
const DeviceType::Ptr DeviceType::LAPTOP = add_type(405, "Laptop");
const DeviceType::Ptr DeviceType::PHOTOCOPIER = add_type(406, "Photocopier");
const DeviceType::Ptr DeviceType::PRINTER = add_type(407, "Printer");
const DeviceType::Ptr DeviceType::OTHER_IT_DEVICE = add_type(408, "Other IT device");
const DeviceType::Ptr DeviceType::XBOX_ONE = add_type(409, "Xbox One");
const DeviceType::Ptr DeviceType::CABLE_RECEIVER = add_type(410, "Cable Receiver");

const DeviceType::Ptr DeviceType::EMOS = add_type(501, "EMOS");
const DeviceType::Ptr DeviceType::WELL_PUMP = add_type(502, "Well pump");

const std::string DeviceType::str() {
    std::ostringstream oss;
    oss << _name;
    return oss.str();
}

bool DeviceType::operator ==(const DeviceType& d) {
    return _id == d.id();
}

bool DeviceType::operator !=(const DeviceType& d) {
    return not operator==(d);
}

std::map<int, DeviceType::Ptr> DeviceType::get_all() {
    return _all;
}

DeviceType::Ptr DeviceType::get_by_id(int id) {

    std::map<int, boost::shared_ptr<klio::DeviceType> >::const_iterator it = _all.find(id);

    if (it == _all.end()) {
        std::ostringstream oss;
        oss << "Invalid device type id: " << id;
        throw DataFormatException(oss.str());

    } else {
        return it->second;
    }
}

DeviceType::Ptr DeviceType::add_type(const int id, const std::string& name) {

    DeviceType::_all[id] = klio::DeviceType::Ptr(new klio::DeviceType(id, name));
    return DeviceType::_all[id];
}
