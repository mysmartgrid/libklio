/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>, 2011
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
 *
 */

#include <sstream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/filesystem.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor-factory.hpp>
#include <libklio/octave-exporter.hpp>

namespace bfs = boost::filesystem;
namespace po = boost::program_options;

int main(int argc, char** argv) {

    try {
        std::ostringstream oss;
        oss << "the device being measured by the sensor" << std::endl;

        std::map<int, klio::DeviceType::Ptr> types = klio::DeviceType::get_all();
        std::map<int, klio::DeviceType::Ptr>::const_iterator it;

        for (it = types.begin(); it != types.end(); it++) {
            klio::DeviceType::Ptr type = (*it).second;
            oss << type->id() << " - " << type->name() << std::endl;
        }
        std::string device_type_help = std::string(oss.str());

        oss.str(std::string());
        oss << "Usage: " << argv[0] << " ACTION [additional options]";
        po::options_description desc(oss.str());
        desc.add_options()
                ("help,h", "produce help message")
                ("version,v", "print libklio version and exit")
                ("action,a", po::value<std::string>(), "Valid actions: create, list, info, addreading, change-description, dump, sync")
                ("storefile,s", po::value<std::string>(), "the data store to use")
                ("id,i", po::value<std::string>(), "the external id of the sensor")
                ("unit,u", po::value<std::string>(), "the unit of the sensor")
                ("timezone,z", po::value<std::string>(), "the timezone of the sensor")
                ("description,d", po::value<std::string>(), "the description of the sensor")
                ("devicetype,c", po::value<int>(), device_type_help.c_str())
                ("reading,r", po::value<double>(), "the reading to add")
                ("timestamp,t", po::value<long>(), "a timestamp to use for the reading")
                ("sourcestorefile,y", po::value<std::string>(), "the data store to use as synchronization source")
                ;
        po::positional_options_description p;
        p.add("action", 1);
        p.add("storefile", 1);
        p.add("id", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                options(desc).positional(p).run(), vm);
        po::notify(vm);

        // Begin processing of commandline parameters.
        std::string action;
        std::string storefile;

        if (vm.count("help")) {
            std::cout << desc << std::endl;
            return 1;
        }

        if (vm.count("version")) {
            klio::VersionInfo::Ptr vi(new klio::VersionInfo());
            std::cout << "klio library version " << vi->getVersion() << std::endl;
            return 0;
        }

        if (!vm.count("storefile")) {
            std::cerr << "You must specify a store to work on." << std::endl;
            return 1;
        } else {
            storefile = vm["storefile"].as<std::string>();
        }

        if (!vm.count("action")) {
            std::cerr << "You must specify an action." << std::endl;
            return 1;
        } else {
            //action=boost::algorithm::to_lower(vm["action"].as<std::string>());
            action = (vm["action"].as<std::string>());
        }
        bfs::path db(storefile);
        if (!bfs::exists(db)) {
            std::cerr << "File " << db << " does not exist, exiting." << std::endl;
            return 2;
        }
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());
        klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

        if (boost::iequals(action, std::string("CREATE"))) {
            //CREATE sensor command 

            if (!vm.count("id") || !vm.count("unit") || !vm.count("timezone")) {
                std::cerr << "You must specify id, unit, and timezone in order " <<
                        "to create a new sensor." << std::endl;
                return 1;
            }
            std::string sensor_id(vm["id"].as<std::string>());
            std::string sensor_unit(vm["unit"].as<std::string>());
            std::string sensor_timezone(vm["timezone"].as<std::string>());

            klio::DeviceType::Ptr device_type = vm.count("devicetype") ?
                    klio::DeviceType::get_by_id(vm["devicetype"].as<int>()) :
                    klio::DeviceType::UNKNOWN_DEVICE;

            std::string sensor_description = vm.count("description") > 0 ?
                    vm["description"].as<std::string>() : "";

            klio::Sensor::Ptr new_sensor(sensor_factory->createSensor(
                    sensor_id, sensor_id, sensor_description, sensor_unit, sensor_timezone, device_type));
            try {
                klio::Store::Ptr store(factory->open_sqlite3_store(db));
                std::cout << "opened store: " << store->str() << std::endl;

                store->add_sensor(new_sensor);
                std::cout << "added: " << new_sensor->str() << std::endl;

            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

        } else if (boost::iequals(action, std::string("LIST"))) {
            //LIST sensors command

            try {
                klio::Store::Ptr store(factory->open_sqlite3_store(db));
                std::cout << "opened store: " << store->str() << std::endl;

                std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();
                std::cout << "Found " << uuids.size() << " sensor(s) in the database." << std::endl;

                std::vector<klio::Sensor::uuid_t>::iterator it;
                for (it = uuids.begin(); it < uuids.end(); it++) {

                    klio::Sensor::Ptr sensor(store->get_sensor(*it));
                    std::cout << " * " << sensor->str() << std::endl;
                }

            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

        } else if (boost::iequals(action, std::string("INFO"))) {
            //INFO sensor command

            try {
                if (!vm.count("id")) {
                    std::cout << "You must specify the external id of the sensor." << std::endl;
                    return 2;
                }

                std::string sensor_id(vm["id"].as<std::string>());
                klio::Store::Ptr store(factory->open_sqlite3_store(db));
                std::cout << "opened store: " << store->str() << std::endl;

                std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_external_id(sensor_id);
                std::vector<klio::Sensor::Ptr>::iterator it;

                for (it = sensors.begin(); it < sensors.end(); it++) {
                    klio::Sensor::Ptr sensor = *it;

                    std::cout << "Info for sensor " << sensor->name() << std::endl;
                    std::cout << " * description:" << sensor->description() << std::endl;
                    std::cout << " * uuid:" << sensor->uuid() << std::endl;
                    std::cout << " * unit:" << sensor->unit() << std::endl;
                    std::cout << " * timezone:" << sensor->timezone() << std::endl;
                    std::cout << " * device type:" << sensor->device_type() << std::endl;
                    std::cout << " * " << store->get_num_readings(sensor)
                            << " readings stored" << std::endl;
                }
            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

        } else if (boost::iequals(action, std::string("ADDREADING"))) {
            //ADDREADING to sensor command

            try {
                if (!vm.count("id") || !vm.count("reading")) {
                    std::cout << "You must specify the external id of the sensor and the value to add." << std::endl;
                    return 2;
                }

                std::string sensor_id(vm["id"].as<std::string>());
                double reading = vm["reading"].as<double>();

                klio::Store::Ptr store(factory->open_sqlite3_store(db));
                std::cout << "opened store: " << store->str() << std::endl;

                std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_external_id(sensor_id);
                std::vector<klio::Sensor::Ptr>::iterator it;

                for (it = sensors.begin(); it < sensors.end(); it++) {
                    klio::Sensor::Ptr sensor = *it;

                    klio::TimeConverter::Ptr tc(new klio::TimeConverter());
                    klio::timestamp_t timestamp = tc->get_timestamp();

                    if (vm.count("timestamp")) {
                        klio::timestamp_t ts = vm["timestamp"].as<long>();
                        timestamp = tc->convert_to_epoch(ts);
                    }
                    store->add_reading(sensor, timestamp, reading);
                    std::cout << "Added reading to sensor "
                            << sensor->name() << std::endl;
                }
            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

        } else if (boost::iequals(action, std::string("CHANGE-DESCRIPTION"))) {
            //Change description of sensor 

            try {
                if (!vm.count("id") || !vm.count("description")) {
                    std::cout << "You must specify the id and the new description of the sensor." << std::endl;
                    return 2;
                }
                std::string sensor_id(vm["id"].as<std::string>());
                std::string description(vm["description"].as<std::string>());

                klio::Store::Ptr store(factory->open_sqlite3_store(db));
                std::cout << "opened store: " << store->str() << std::endl;

                std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_external_id(sensor_id);
                std::vector<klio::Sensor::Ptr>::iterator it;

                for (it = sensors.begin(); it < sensors.end(); it++) {
                    klio::Sensor::Ptr sensor = *it;

                    //FIXME: set description
                    store->update_sensor(sensor);
                }
            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

        } else if (boost::iequals(action, std::string("DUMP"))) {
            //DUMP sensor reading command

            try {
                if (!vm.count("id")) {
                    std::cout << "You must specify the external id of the sensor." << std::endl;
                    return 2;
                }
                std::string sensor_id(vm["id"].as<std::string>());

                klio::Store::Ptr store(factory->open_sqlite3_store(db));
                std::cout << "opened store: " << store->str() << std::endl;

                std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_external_id(sensor_id);
                std::vector<klio::Sensor::Ptr>::iterator it;

                for (it = sensors.begin(); it < sensors.end(); it++) {
                    klio::Sensor::Ptr sensor = *it;

                    klio::readings_t_Ptr readings = store->get_all_readings(sensor);
                    klio::readings_it_t it;
                    std::cout << "timestamp\treading" << std::endl;

                    for (it = readings->begin(); it != readings->end(); it++) {
                        klio::timestamp_t ts1 = (*it).first;
                        double val1 = (*it).second;
                        std::cout << ts1 << "\t" << val1 << std::endl;
                    }
                }
            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

        } else if (boost::iequals(action, std::string("octscript"))) {
            //Export to octave script file command

            try {
                if (!vm.count("id")) {
                    std::cout << "You must specify the id of the sensor." << std::endl;
                    return 2;
                }
                std::string sensor_id(vm["id"].as<std::string>());

                klio::Store::Ptr store(factory->open_sqlite3_store(db));
                std::cout << "opened store: " << store->str() << std::endl;

                std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_external_id(sensor_id);
                std::vector<klio::Sensor::Ptr>::iterator it;

                for (it = sensors.begin(); it < sensors.end(); it++) {
                    klio::Sensor::Ptr sensor = *it;

                    klio::readings_t_Ptr readings = store->get_all_readings(sensor);
                    // create output stream
                    klio::Exporter::Ptr octexporter(new klio::OctaveExporter(std::cout));
                    octexporter->process(readings, sensor->name(), sensor->description());
                }
            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

        } else if (boost::iequals(action, std::string("SYNC"))) {
            //SYNC - Sensor readings synchronization command

            try {
                if (!vm.count("id") || !vm.count("sourcestorefile")) {
                    std::cout << "You must specify the external id of the sensor and the source store file path." << std::endl;
                    return 2;
                }

                std::string sensor_id(vm["id"].as<std::string>());
                std::string sourcestorefile(vm["sourcestorefile"].as<std::string>());

                klio::Store::Ptr store(factory->open_sqlite3_store(db));
                std::cout << "opened store: " << store->str() << std::endl;

                bfs::path sourcedb(sourcestorefile);
                klio::Store::Ptr sourcestore(factory->open_sqlite3_store(sourcedb));
                std::cout << "opened source store: " << sourcestore->str() << std::endl;

                std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_external_id(sensor_id);
                std::vector<klio::Sensor::Ptr>::iterator it;

                for (it = sensors.begin(); it < sensors.end(); it++) {
                    klio::Sensor::Ptr sensor = *it;

                    store->sync_readings(sensor, sourcestore);
                }

            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }
        } else {
            //UNKNOWN command
            std::cerr << "Unknown command " << action << std::endl;
            return 1;
        }

    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
    }
    return 0;
}
