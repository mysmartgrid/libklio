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

#include <libklio/common.hpp>
#include <sstream>
#include <libklio/store.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor.hpp>
#include <libklio/sensorfactory.hpp>
#include <libklio/exporter.hpp>
#include <libklio/octave_exporter.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem; 
namespace po = boost::program_options;

int main(int argc,char** argv) {

  try {
    std::ostringstream oss;
    oss << "Usage: " << argv[0] << " ACTION [additional options]";
    po::options_description desc(oss.str());
    desc.add_options()
      ("help,h", "produce help message")
      ("version,v", "print libklio version and exit")
      ("action,a", po::value<std::string>(), "Valid actions: create, list, info, addreading, change-description, dump")
      ("storefile,s", po::value<std::string>(), "the data store to use")
      ("id,i", po::value<std::string>(), "the id of the sensor")
      ("unit,u", po::value<std::string>(), "the unit of the sensor")
      ("timezone,z", po::value<std::string>(), "the timezone of the sensor")
      ("description,d", po::value<std::string>(), "the description of the sensor")
      ("reading,r", po::value<double>(), "the reading to add")
      ("timestamp,t", po::value<long>(), "a timestamp to use for the reading")
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

    if (! vm.count("storefile")) {
      std::cerr << "You must specify a store to work on." << std::endl;
      return 1;
    } else {
      storefile=vm["storefile"].as<std::string>();
    }

    if (! vm.count("action")) {
      std::cerr << "You must specify an action." << std::endl;
      return 1;
    } else {
      //action=boost::algorithm::to_lower(vm["action"].as<std::string>());
      action=(vm["action"].as<std::string>());
    }
    bfs::path db(storefile);
    if (! bfs::exists(db)) {
      std::cerr << "File " << db << " does not exist, exiting." << std::endl;
      return 2;
    }
    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

    /**
     * CREATE sensor command
     */
    if (boost::iequals(action, std::string("CREATE"))) {
      if ( !vm.count("id") || !vm.count("unit") || !vm.count("timezone")) {
        std::cerr << "You must specify id, unit and timezone in order " <<
          "to create a new sensor."<< std::endl;
        return 1;
      }
      std::string sensor_id(vm["id"].as<std::string>());
      std::string sensor_unit(vm["unit"].as<std::string>());
      std::string sensor_timezone(vm["timezone"].as<std::string>());
      klio::Sensor::Ptr new_sensor(sensor_factory->createSensor(
            sensor_id, sensor_unit, sensor_timezone)); 
      try {
        klio::Store::Ptr store(factory->openStore(klio::SQLITE3, db));
        std::cout << "opened store: " << store->str() << std::endl;
        store->addSensor(new_sensor);
        std::cout << "added: " << new_sensor->str() << std::endl;
      } catch (klio::StoreException const& ex) {
        std::cout << "Failed to create: " << ex.what() << std::endl;
      }
    }

    /**
     * LIST sensors command
     */
    else if (boost::iequals(action, std::string("LIST"))) {
      try {
        klio::Store::Ptr store(factory->openStore(klio::SQLITE3, db));
        std::cout << "opened store: " << store->str() << std::endl;
        std::vector<klio::Sensor::uuid_t> uuids = store->getSensorUUIDs();
        std::cout << "Found " << uuids.size() 
          << " sensor(s) in the database." << std::endl;
        std::vector<klio::Sensor::uuid_t>::iterator it;
        for(  it = uuids.begin(); it < uuids.end(); it++) {
          klio::Sensor::Ptr loadedSensor(store->getSensor(*it));
          std::cout << " * " << loadedSensor->str() << std::endl;
        }
      } catch (klio::StoreException const& ex) {
        std::cout << "Failed to create: " << ex.what() << std::endl;
      }
    }

    /**
     * INFO sensor command
     */
    else if (boost::iequals(action, std::string("INFO"))) {
      try {
        if ( !vm.count("id") ) {
          std::cout << "You must specify the id of the sensor." << std::endl;
          return 2;
        }
        std::string sensor_id(vm["id"].as<std::string>());
        klio::Store::Ptr store(factory->openStore(klio::SQLITE3, db));
        std::cout << "opened store: " << store->str() << std::endl;
        std::vector<klio::Sensor::uuid_t> uuids = store->getSensorUUIDs();
        std::vector<klio::Sensor::uuid_t>::iterator it;
        for(  it = uuids.begin(); it < uuids.end(); it++) {
          klio::Sensor::Ptr loadedSensor(store->getSensor(*it));
          if (boost::iequals(loadedSensor->name(), sensor_id)) {
            std::cout << "Info for sensor " << loadedSensor->name() << std::endl;
            std::cout << " * description:" << loadedSensor->description() << std::endl;
            std::cout << " * uuid:" << loadedSensor->uuid() << std::endl;
            std::cout << " * unit:" << loadedSensor->unit() << std::endl;
            std::cout << " * timezone:" << loadedSensor->timezone() << std::endl;
            std::cout << " * " <<  store->get_num_readings(loadedSensor)  
              << " readings stored" << std::endl;
          }
        }
      } catch (klio::StoreException const& ex) {
        std::cout << "Failed to create: " << ex.what() << std::endl;
      }
    }
    
    /**
     * ADDREADING to sensor command
     */
    else if (boost::iequals(action, std::string("ADDREADING"))) {
      try {
        if ( !vm.count("id") || !vm.count("reading") ) {
          std::cout << "You must specify the id of the sensor and the value to add." 
            << std::endl;
          return 2;
        }
        std::string sensor_id(vm["id"].as<std::string>());
        double reading=vm["reading"].as<double>();
        klio::Store::Ptr store(factory->openStore(klio::SQLITE3, db));
        std::cout << "opened store: " << store->str() << std::endl;
        std::vector<klio::Sensor::uuid_t> uuids = store->getSensorUUIDs();
        std::vector<klio::Sensor::uuid_t>::iterator it;
        for(  it = uuids.begin(); it < uuids.end(); it++) {
          klio::Sensor::Ptr loadedSensor(store->getSensor(*it));
          if (boost::iequals(loadedSensor->name(), sensor_id)) {
            klio::TimeConverter::Ptr tc(new klio::TimeConverter());
            klio::timestamp_t timestamp=tc->get_timestamp();
            if (vm.count("timestamp")) {
              klio::timestamp_t ts=vm["timestamp"].as<long>();
              timestamp= tc->convert_to_epoch(ts);
            }
            store->add_reading(loadedSensor, timestamp, reading);
            std::cout << "Added reading to sensor " 
              << loadedSensor->name() << std::endl;
          }
        }
      } catch (klio::StoreException const& ex) {
        std::cout << "Failed to create: " << ex.what() << std::endl;
      }
    }
    
    /**
     * Change description of sensor 
     */
    else if (boost::iequals(action, std::string("CHANGE-DESCRIPTION"))) {
      try {
        if ( !vm.count("id") || !vm.count("description") ) {
          std::cout << "You must specify the id and the new description of the sensor." 
            << std::endl;
          return 2;
        }
        std::string sensor_id(vm["id"].as<std::string>());
        std::string desc(vm["description"].as<std::string>());
        klio::Store::Ptr store(factory->openStore(klio::SQLITE3, db));
        std::cout << "opened store: " << store->str() << std::endl;
        std::vector<klio::Sensor::uuid_t> uuids = store->getSensorUUIDs();
        std::vector<klio::Sensor::uuid_t>::iterator it;
        for(  it = uuids.begin(); it < uuids.end(); it++) {
          klio::Sensor::Ptr loadedSensor(store->getSensor(*it));
          if (boost::iequals(loadedSensor->name(), sensor_id)) {
            store->add_description(loadedSensor, desc);
          }
        }
      } catch (klio::StoreException const& ex) {
        std::cout << "Failed to create: " << ex.what() << std::endl;
      }
    }

    /**
     * Dump sensor reading command
     */
    else if (boost::iequals(action, std::string("DUMP"))) {
      try {
        if ( !vm.count("id")  ) {
          std::cout << "You must specify the id of the sensor." 
            << std::endl;
          return 2;
        }
        std::string sensor_id(vm["id"].as<std::string>());
        klio::Store::Ptr store(factory->openStore(klio::SQLITE3, db));
        std::cout << "opened store: " << store->str() << std::endl;
        std::vector<klio::Sensor::uuid_t> uuids = store->getSensorUUIDs();
        std::vector<klio::Sensor::uuid_t>::iterator it;
        for(  it = uuids.begin(); it < uuids.end(); it++) {
          klio::Sensor::Ptr loadedSensor(store->getSensor(*it));
          if (boost::iequals(loadedSensor->name(), sensor_id)) {
            klio::readings_t_Ptr readings = store->get_all_readings(loadedSensor);
            klio::readings_it_t it;
            std::cout << "timestamp\treading" << std::endl;
            for(  it = readings->begin(); it != readings->end(); it++) {
              klio::timestamp_t ts1=(*it).first;
              double val1=(*it).second;
              std::cout << ts1 << "\t" << val1 << std::endl;
            }
          }
        }
      } catch (klio::StoreException const& ex) {
        std::cout << "Failed to create: " << ex.what() << std::endl;
      }
    }

    /**
     * Export to octave script file command
     */
    else if (boost::iequals(action, std::string("octscript"))) {
      try {
        if ( !vm.count("id")  ) {
          std::cout << "You must specify the id of the sensor." 
            << std::endl;
          return 2;
        }
        std::string sensor_id(vm["id"].as<std::string>());
        klio::Store::Ptr store(factory->openStore(klio::SQLITE3, db));
        std::cout << "opened store: " << store->str() << std::endl;
        std::vector<klio::Sensor::uuid_t> uuids = store->getSensorUUIDs();
        std::vector<klio::Sensor::uuid_t>::iterator it;
        for(  it = uuids.begin(); it < uuids.end(); it++) {
          klio::Sensor::Ptr loadedSensor(store->getSensor(*it));
          if (boost::iequals(loadedSensor->name(), sensor_id)) {
            klio::readings_t_Ptr readings = store->get_all_readings(loadedSensor);
            // create output stream
            klio::Exporter::Ptr octexporter(new klio::OctaveExporter(
                  std::cout));
            octexporter->process(readings, 
                loadedSensor->name(), loadedSensor->description());
          }
        }
      } catch (klio::StoreException const& ex) {
        std::cout << "Failed to create: " << ex.what() << std::endl;
      }
    }

    /**
     * UNKNOWN command
     */
    else {
      std::cerr << "Unknown command " << action << std::endl;
      return 1;
    }

  } catch(std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch(...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }
  return 0;

}
