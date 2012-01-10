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
#include <fstream>
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
namespace po = boost::program_options;

int main(int argc,char** argv) {

  try {
    std::ostringstream oss;
    oss << "Usage: " << argv[0] << " ACTION STORE ID [additional options]";
    po::options_description desc(oss.str());
    desc.add_options()
      ("help,h", "produce help message")
      ("version,v", "print libklio version and exit")
      ("action,a", po::value<std::string>(), "Valid actions: table, octave, json")
      ("storefile,s", po::value<std::string>(), "the data store to use")
      ("outputfile,o", po::value<std::string>(), "the output file to use")
      ("id,i", po::value<std::string>(), "the id of the sensor")
      ("lasthours,l", po::value<long>(), "export the last N hours of data")
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
      action=(vm["action"].as<std::string>());
    }
    bfs::path db(storefile);
    if (! bfs::exists(db)) {
      std::cerr << "File " << db << " does not exist, exiting." << std::endl;
      return 2;
    }

    // Where to write the exports to
    std::ostream* outputstream = &std::cout;
    if (! vm.count("outputfile")) {
      std::cerr << "Using stdout" << std::endl;
    } else {
      std::string outputfile=vm["outputfile"].as<std::string>();
      bfs::path of(outputfile);
      if (bfs::exists(of)) {
        std::cerr << "File " << of << " does exist, exiting." << std::endl;
        return 2;
      } else {
        outputstream = new std::ofstream(of.c_str());
      }
    }

    klio::StoreFactory::Ptr factory(new klio::StoreFactory()); 
    klio::SensorFactory::Ptr sensor_factory(new klio::SensorFactory());

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


          /**
           * Dump sensor reading command
           */
          if (boost::iequals(action, std::string("TABLE"))) {
            klio::readings_it_t it;
            *outputstream << "timestamp\treading" << std::endl;
            for(  it = readings->begin(); it != readings->end(); it++) {
              klio::timestamp_t ts1=(*it).first;
              double val1=(*it).second;
              *outputstream << ts1 << "\t" << val1 << std::endl;
            }
          }

          /**
           * Export to octave script file command
           */
          else if (boost::iequals(action, std::string("OCTAVE"))) {
            // create output stream
            klio::Exporter::Ptr octexporter(new klio::OctaveExporter(
                  *outputstream));
            octexporter->process(readings, 
                loadedSensor->name(), loadedSensor->description());

          }

          /**
           * UNKNOWN command
           */
          else {
            std::cerr << "Unknown command " << action << std::endl;
            return 1;
          }
        }
      }
    } catch (klio::StoreException const& ex) {
      std::cout << "Failed to export: " << ex.what() << std::endl;
    }

    if (outputstream != &std::cout)
      delete outputstream;

  } catch(std::exception& e) {
    std::cerr << "error: " << e.what() << std::endl;
    return 1;
  } catch(...) {
    std::cerr << "Exception of unknown type!" << std::endl;
  }
  return 0;

}
