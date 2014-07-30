/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>, 2011
 *                       Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2014
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
#include <fstream>
#include <boost/uuid/uuid_io.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <libklio/store-factory.hpp>
#include <libklio/sensor-factory.hpp>
#include <libklio/csv-importer.hpp>
#include <libklio/local-time.hpp>

namespace po = boost::program_options;
using namespace boost::gregorian;

int main(int argc, char** argv) {

    try {
        std::ostringstream oss;
        oss << "Usage: " << argv[0] << " ACTION STORE ID [additional options]";
        po::options_description desc(oss.str());
        desc.add_options()
                ("help,h", "produce help message")
                ("version,v", "print libklio version and exit")
                ("action,a", po::value<std::string>(), "Valid actions: csv")
                ("separators,c", po::value<std::string>(), "the characters used in the readings file to separate columns")
                ("storefile,s", po::value<std::string>(), "the data store to use")
                ("inputfile,f", po::value<std::string>(), "the readings file to import")
                ("id,i", po::value<std::string>(), "the internal id of the sensor (i.e. 3e3b6ef2-d960-4677-8845-1f52977b16d6)")
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
            action = (vm["action"].as<std::string>());
        }

        bfs::path db(storefile);
        if (!bfs::exists(db)) {
            std::cerr << "File " << db << " does not exist, exiting." << std::endl;
            return 2;
        }

        // Where to read the measurements from
        if (!vm.count("inputfile")) {
            std::cerr << "Input file not informed." << std::endl;
            return 2;
        }
        bfs::path inputfile(vm["inputfile"].as<std::string>());

        if (!bfs::exists(inputfile)) {
            std::cerr << "File " << inputfile << " does not exist, exiting." << std::endl;
            return 2;
        }

        std::string separators = vm.count("separators") ? vm["separators"].as<std::string>() : ",";

        if (!vm.count("id")) {
            std::cout << "You must specify the internal id of the sensor." << std::endl;
            return 2;
        }
        std::string sensor_id(vm["id"].as<std::string>());

        std::ifstream* inputstream = new std::ifstream(inputfile.c_str());
        klio::StoreFactory::Ptr factory(new klio::StoreFactory());

        klio::Store::Ptr store(factory->open_sqlite3_store(db));
        std::cout << "opened store: " << store->str() << std::endl;

        try {
            std::vector<klio::Sensor::Ptr> sensors = store->get_sensors_by_external_id(sensor_id);

            if (sensors.empty()) {
                std::cout << "Sensor " << sensor_id << " not found. Aborting." << std::endl;

            } else {
                klio::Sensor::Ptr sensor = *(sensors.begin());
                std::cout << "Found sensor \"" << sensor->name() << "\"" << std::endl;

                if (boost::iequals(action, std::string("CSV"))) {
                    //CSV command

                    klio::Importer::Ptr importer(new klio::CSVImporter(*inputstream, separators));
                    klio::readings_t_Ptr readings = importer->process();

                    store->update_readings(sensor, *readings);

                } else {
                    //UNKNOWN command
                    std::cerr << "Unknown command " << action << std::endl;
                    return 1;
                }
            }

        } catch (klio::StoreException const& ex) {
            std::cout << "Failed to import: " << ex.what() << std::endl;
        }

        inputstream->close();
        delete inputstream;
        store->close();

    } catch (std::exception& e) {
        std::cerr << "error: " << e.what() << std::endl;
        return 1;

    } catch (...) {
        std::cerr << "Exception of unknown type!" << std::endl;
    }
    return 0;
}
