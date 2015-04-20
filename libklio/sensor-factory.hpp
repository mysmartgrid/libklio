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

#ifndef LIBKLIO_SENSORFACTORY_HPP
#define LIBKLIO_SENSORFACTORY_HPP 1

#include <boost/uuid/uuid_generators.hpp>
#include <boost/shared_ptr.hpp>
#include <libklio/common.hpp>
#include <libklio/sensor.hpp>
#include <libklio/device-type.hpp>

namespace klio {

    class SensorFactory {
    public:
        typedef boost::shared_ptr<SensorFactory> Ptr;

        SensorFactory() {
        };

        Sensor::Ptr createSensor(
                const std::string& external_id,
                const std::string& name,
                const std::string& unit,
                const std::string& timezone
                );

        Sensor::Ptr createSensor(
                const std::string& external_id,
                const std::string& name,
                const std::string& unit,
                const std::string& timezone,
                const DeviceType::Ptr device_type
                );

        Sensor::Ptr createSensor(
                const std::string& external_id,
                const std::string& name,
                const std::string& description,
                const std::string& unit,
                const std::string& timezone,
                const DeviceType::Ptr device_type
                );

        Sensor::Ptr createSensor(
                const std::string& uuid_string,
                const std::string& external_id,
                const std::string& name,
                const std::string& unit,
                const std::string& timezone
                );

        Sensor::Ptr createSensor(
                const std::string& uuid_string,
                const std::string& external_id,
                const std::string& name,
                const std::string& description,
                const std::string& unit,
                const std::string& timezone
                );

        Sensor::Ptr createSensor(
                const std::string& uuid_string,
                const std::string& external_id,
                const std::string& name,
                const std::string& description,
                const std::string& unit,
                const std::string& timezone,
                const DeviceType::Ptr device_type
                );

        Sensor::Ptr createSensor(
                const Sensor::uuid_t& uuid,
                const std::string& external_id,
                const std::string& name,
                const std::string& description,
                const std::string& unit,
                const std::string& timezone
                );

        Sensor::Ptr createSensor(
                const Sensor::uuid_t& uuid,
                const std::string& external_id,
                const std::string& name,
                const std::string& description,
                const std::string& unit,
                const std::string& timezone,
                const DeviceType::Ptr device_type
                );

        virtual ~SensorFactory() {
        };

    private:
        SensorFactory(const SensorFactory& original);
        SensorFactory& operator=(const SensorFactory& rhs);

        static boost::uuids::random_generator _gen_uuid;
    };
};

#endif /* LIBKLIO_SENSORFACTORY_HPP */
