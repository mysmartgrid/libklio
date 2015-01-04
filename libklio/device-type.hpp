/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>,    2010
 *                       Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2013
 *
 * libklio is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * libklio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with libklio. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef LIBKLIO_DEVICE_TYPE_HPP
#define LIBKLIO_DEVICE_TYPE_HPP 1

#include <map>
#include <sstream>
#include <boost/shared_ptr.hpp>
#include <libklio/common.hpp>

namespace klio {

    class DeviceType {
    public:
        typedef boost::shared_ptr<DeviceType> Ptr;

        const static DeviceType::Ptr UNKNOWN_DEVICE;
        const static DeviceType::Ptr DISABLED_DEVICE;

        const static DeviceType::Ptr SINGLE_PHASE_ENERGY_CONSUMPTION;
        const static DeviceType::Ptr TRIPLE_PHASE_ENERGY_CONSUMPTION;
        const static DeviceType::Ptr SINGLE_PHASE_ENERGY_PRODUCTION;
        const static DeviceType::Ptr TRIPLE_PHASE_ENERGY_PRODUCTION;

        const static DeviceType::Ptr HEATING_CIRCULATING_PUMP;
        const static DeviceType::Ptr HEATING_PUMP;
        const static DeviceType::Ptr INFRARED_RADIATOR;
        const static DeviceType::Ptr SINGLE_PHASE_RADIATOR;
        const static DeviceType::Ptr TRIPLE_PHASE_RADIATOR;
        const static DeviceType::Ptr AIR_CONDITIONER;

        const static DeviceType::Ptr FRIDGE;
        const static DeviceType::Ptr FREEZER;
        const static DeviceType::Ptr FRIDGE_FREEZER_COMBINATION;
        const static DeviceType::Ptr TOP_LID_FREEZER;
        const static DeviceType::Ptr WASHMACHINE;
        const static DeviceType::Ptr DRIER;
        const static DeviceType::Ptr DISHWASHER;
        const static DeviceType::Ptr EXPRESSO_MACHINE;
        const static DeviceType::Ptr COFFEE_MACHINE;
        const static DeviceType::Ptr TOASTER;
        const static DeviceType::Ptr KETTLE;
        const static DeviceType::Ptr MICROWAVE;
        const static DeviceType::Ptr HAIR_DRIER;
        const static DeviceType::Ptr AQUARIUM;
        const static DeviceType::Ptr KITCHEN_AID;

        const static DeviceType::Ptr TV;
        const static DeviceType::Ptr RADIO;
        const static DeviceType::Ptr LAMP;
        const static DeviceType::Ptr COMPUTER;
        const static DeviceType::Ptr LAPTOP;
        const static DeviceType::Ptr PHOTOCOPIER;
        const static DeviceType::Ptr PRINTER;
        const static DeviceType::Ptr OTHER_IT_DEVICE;
        const static DeviceType::Ptr XBOX_ONE;
        const static DeviceType::Ptr CABLE_RECEIVER;

        const static DeviceType::Ptr EMOS;
        const static DeviceType::Ptr WELL_PUMP;

        static std::map<int, DeviceType::Ptr> get_all();
        static DeviceType::Ptr get_by_id(const int id);

        virtual ~DeviceType() {
        };

        // standard device type methods

        const int id() const {
            return _id;
        };

        const std::string name() const {
            return _name;
        };

        const std::string str();

        bool operator ==(const DeviceType& d);
        bool operator !=(const DeviceType& d);

    private:

        DeviceType(const int id, const std::string& name) :
        _id(id),
        _name(name) {
        };

        DeviceType(const DeviceType& original);
        DeviceType& operator =(const DeviceType& d);

        void id(const int id) {
            _id = id;
        }

        void name(const std::string& name) {
            _name = name;
        }

        static DeviceType::Ptr add_type(const int id, const std::string& name);

        int _id;
        std::string _name;
        static std::map<int, DeviceType::Ptr> _all;
    };
};

#endif /* LIBKLIO_DEVICE_TYPE_HPP */
