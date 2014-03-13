#include <sstream>
#include <boost/assign.hpp>
#include "device-type.hpp"


using namespace klio;

const DeviceType::Ptr DeviceType::UNKNOWN_DEVICE(new DeviceType(000, "Unknown Device"));
const DeviceType::Ptr DeviceType::DISABLED_DEVICE(new DeviceType(001, "Disabled Device"));

const DeviceType::Ptr DeviceType::SINGLE_PHASE_ENERGY_CONSUMPTION(new DeviceType(100, "Single-phase Energy Consumption"));
const DeviceType::Ptr DeviceType::TRIPLE_PHASE_ENERGY_CONSUMPTION(new DeviceType(101, "Triple-phase Energy Consumption"));
const DeviceType::Ptr DeviceType::SINGLE_PHASE_ENERGY_PRODUCTION(new DeviceType(102, "Single-phase Energy Production"));
const DeviceType::Ptr DeviceType::TRIPLE_PHASE_ENERGY_PRODUCTION(new DeviceType(103, "Triple-phase Energy Production"));

const DeviceType::Ptr DeviceType::HEATING_CIRCULATING_PUMP(new DeviceType(200, "Heating Circulating Pump"));
const DeviceType::Ptr DeviceType::HEATING_PUMP(new DeviceType(201, "Heating Pump"));
const DeviceType::Ptr DeviceType::INFRARED_RADIATOR(new DeviceType(202, "Infrared Radiator"));
const DeviceType::Ptr DeviceType::SINGLE_PHASE_RADIATOR(new DeviceType(203, "Single-phase Radiator"));
const DeviceType::Ptr DeviceType::TRIPLE_PHASE_RADIATOR(new DeviceType(204, "Triple-phase Radiator"));
const DeviceType::Ptr DeviceType::AIR_CONDITIONER(new DeviceType(205, "Air Conditioner"));

const DeviceType::Ptr DeviceType::FRIDGE(new DeviceType(300, "Fridge"));
const DeviceType::Ptr DeviceType::FREEZER(new DeviceType(301, "Freezer"));
const DeviceType::Ptr DeviceType::FRIDGE_FREEZER_COMBINATION(new DeviceType(302, "Fridge-freezer Combination"));
const DeviceType::Ptr DeviceType::TOP_LID_FREEZER(new DeviceType(303, "Top-lid Freezer"));
const DeviceType::Ptr DeviceType::WASHMACHINE(new DeviceType(304, "Wash Machine"));
const DeviceType::Ptr DeviceType::DRIER(new DeviceType(305, "Drier"));
const DeviceType::Ptr DeviceType::DISHWASHER(new DeviceType(306, "Dishwasher"));
const DeviceType::Ptr DeviceType::EXPRESSO_MACHINE(new DeviceType(307, "Expresso Machine"));
const DeviceType::Ptr DeviceType::COFFEE_MACHINE(new DeviceType(308, "Coffee Machine"));
const DeviceType::Ptr DeviceType::TOASTER(new DeviceType(309, "Toaster"));
const DeviceType::Ptr DeviceType::KETTLE(new DeviceType(310, "Kettle"));
const DeviceType::Ptr DeviceType::MICROWAVE(new DeviceType(311, "Microwave"));
const DeviceType::Ptr DeviceType::HAIR_DRIER(new DeviceType(312, "Hair Drier"));
const DeviceType::Ptr DeviceType::AQUARIUM(new DeviceType(313, "Aquarium"));
const DeviceType::Ptr DeviceType::KITCHEN_AID(new DeviceType(314, "Kitchen Aid"));

const DeviceType::Ptr DeviceType::TV(new DeviceType(400, "TV"));
const DeviceType::Ptr DeviceType::RADIO(new DeviceType(401, "Radio"));
const DeviceType::Ptr DeviceType::LAMP(new DeviceType(403, "Lamp"));
const DeviceType::Ptr DeviceType::COMPUTER(new DeviceType(404, "Computer"));
const DeviceType::Ptr DeviceType::LAPTOP(new DeviceType(405, "Laptop"));
const DeviceType::Ptr DeviceType::PHOTOCOPIER(new DeviceType(406, "Photocopier"));
const DeviceType::Ptr DeviceType::PRINTER(new DeviceType(407, "Printer"));
const DeviceType::Ptr DeviceType::OTHER_IT_DEVICE(new DeviceType(408, "Other IT device"));
const DeviceType::Ptr DeviceType::XBOX_ONE(new DeviceType(409, "Xbox One"));
const DeviceType::Ptr DeviceType::CABLE_RECEIVER(new DeviceType(410, "Cable Receiver"));

const std::map<int, DeviceType::Ptr> DeviceType::_all = boost::assign::map_list_of

        (DeviceType::UNKNOWN_DEVICE->id(), DeviceType::UNKNOWN_DEVICE)
(DeviceType::DISABLED_DEVICE->id(), DeviceType::DISABLED_DEVICE)

(DeviceType::SINGLE_PHASE_ENERGY_CONSUMPTION->id(), DeviceType::SINGLE_PHASE_ENERGY_CONSUMPTION)
(DeviceType::TRIPLE_PHASE_ENERGY_CONSUMPTION->id(), DeviceType::TRIPLE_PHASE_ENERGY_CONSUMPTION)
(DeviceType::SINGLE_PHASE_ENERGY_PRODUCTION->id(), DeviceType::SINGLE_PHASE_ENERGY_PRODUCTION)
(DeviceType::TRIPLE_PHASE_ENERGY_PRODUCTION->id(), DeviceType::TRIPLE_PHASE_ENERGY_PRODUCTION)

(DeviceType::HEATING_CIRCULATING_PUMP->id(), DeviceType::HEATING_CIRCULATING_PUMP)
(DeviceType::HEATING_PUMP->id(), DeviceType::HEATING_PUMP)
(DeviceType::INFRARED_RADIATOR->id(), DeviceType::INFRARED_RADIATOR)
(DeviceType::SINGLE_PHASE_RADIATOR->id(), DeviceType::SINGLE_PHASE_RADIATOR)
(DeviceType::TRIPLE_PHASE_RADIATOR->id(), DeviceType::TRIPLE_PHASE_RADIATOR)
(DeviceType::AIR_CONDITIONER->id(), DeviceType::AIR_CONDITIONER)

(DeviceType::FRIDGE->id(), DeviceType::FRIDGE)
(DeviceType::FREEZER->id(), DeviceType::FREEZER)
(DeviceType::FRIDGE_FREEZER_COMBINATION->id(), DeviceType::FRIDGE_FREEZER_COMBINATION)
(DeviceType::TOP_LID_FREEZER->id(), DeviceType::TOP_LID_FREEZER)
(DeviceType::WASHMACHINE->id(), DeviceType::WASHMACHINE)
(DeviceType::DRIER->id(), DeviceType::DRIER)
(DeviceType::DISHWASHER->id(), DeviceType::DISHWASHER)
(DeviceType::EXPRESSO_MACHINE->id(), DeviceType::EXPRESSO_MACHINE)
(DeviceType::COFFEE_MACHINE->id(), DeviceType::COFFEE_MACHINE)
(DeviceType::TOASTER->id(), DeviceType::TOASTER)
(DeviceType::KETTLE->id(), DeviceType::KETTLE)
(DeviceType::MICROWAVE->id(), DeviceType::MICROWAVE)
(DeviceType::HAIR_DRIER->id(), DeviceType::HAIR_DRIER)
(DeviceType::AQUARIUM->id(), DeviceType::AQUARIUM)
(DeviceType::KITCHEN_AID->id(), DeviceType::KITCHEN_AID)

(DeviceType::TV->id(), DeviceType::TV)
(DeviceType::RADIO->id(), DeviceType::RADIO)
(DeviceType::LAMP->id(), DeviceType::LAMP)
(DeviceType::COMPUTER->id(), DeviceType::COMPUTER)
(DeviceType::LAPTOP->id(), DeviceType::LAPTOP)
(DeviceType::PHOTOCOPIER->id(), DeviceType::PHOTOCOPIER)
(DeviceType::PRINTER->id(), DeviceType::PRINTER)
(DeviceType::OTHER_IT_DEVICE->id(), DeviceType::OTHER_IT_DEVICE)
(DeviceType::XBOX_ONE->id(), DeviceType::XBOX_ONE)
(DeviceType::CABLE_RECEIVER->id(), DeviceType::CABLE_RECEIVER);

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

    std::map<int, std::tr1::shared_ptr<klio::DeviceType> >::const_iterator it = _all.find(id);
    
    if (it == _all.end()) {
        std::ostringstream oss;
        oss << "Invalid device type id: " << id;
        throw DataFormatException(oss.str());

    } else {
        return it->second;
    }
}