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
#include <limits>
#include <algorithm>
#include <libklio/common.hpp>
#include <libklio/local-time.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/program_options.hpp>
#include <boost/program_options/positional_options.hpp>
#include <libklio/store-factory.hpp>

namespace po = boost::program_options;

int main(int argc, char** argv) {

    try {
        std::ostringstream oss;
        oss << "Usage: " << argv[0] << " ACTION [additional options]";
        po::options_description desc(oss.str());
        desc.add_options()
                ("help,h", "produce help message")
                ("version,v", "print libklio version and exit")
                ("action,a", po::value<std::string>(), "Valid actions are: create, check, sync")
                ("storefile,s", po::value<std::string>(), "the data store to use")
                ("sourcestore,r", po::value<std::string>(), "the data store to use as source for synchronization")
                ;
        po::positional_options_description p;
        p.add("action", 1);
        p.add("storefile", 1);

        po::variables_map vm;
        po::store(po::command_line_parser(argc, argv).
                options(desc).positional(p).run(), vm);
        po::notify(vm);

        // Begin processing of command line parameters.
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

        /**
         * CREATE action
         */
        if (boost::iequals(action, std::string("create"))) {
            bfs::path db(storefile);
            if (bfs::exists(db)) {
                std::cerr << "File " << db << " already exists, exiting." << std::endl;
                return 2;
            }
            klio::StoreFactory::Ptr factory(new klio::StoreFactory());
            try {
                std::cout << "Attempting to create " << db << std::endl;
                klio::Store::Ptr store(factory->create_sqlite3_store(db));
                std::cout << "Initialized store: " << store->str() << std::endl;

            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

        /**
         * CHECK action
         */
        } else if (boost::iequals(action, std::string("check"))) {
            bfs::path db(storefile);
            if (!bfs::exists(db)) {
                std::cerr << "File " << db << " does not exist, exiting." << std::endl;
                return 2;
            }
            klio::StoreFactory::Ptr factory(new klio::StoreFactory());
            try {
                //  std::cout << "Attempting to open " << db << std::endl;
                klio::Store::Ptr store(factory->open_sqlite3_store(db));
                //  std::cout << "opened store: " << store->str() << std::endl;
                std::vector<klio::Sensor::uuid_t> uuids = store->get_sensor_uuids();
                std::vector<klio::Sensor::uuid_t>::iterator it;
                std::cout << std::setw(8) << "# val";
                std::cout << std::setw(8) << "dt med";
                std::cout << std::setw(8) << "dt min";
                std::cout << std::setw(8) << "dt max";
                std::cout << std::setw(8) << "avg(W)";
                std::cout << std::setw(8) << "min(W)";
                std::cout << std::setw(8) << "max(W)";
                std::cout << std::setw(21) << "first timestamp";
                std::cout << std::setw(21) << "last timestamp";
                std::cout << '\t' << "sensor name" << std::endl;
                for (it = uuids.begin(); it < uuids.end(); it++) {
                    klio::Sensor::Ptr loadedSensor(store->get_sensor(*it));
                    klio::readings_t_Ptr readings;
                    readings = store->get_all_readings(loadedSensor);
                    // loop over all readings and calculate metrics
                    uint64_t num_readings = 0;
                    klio::timestamp_t first_timestamp = 0;
                    klio::timestamp_t last_timestamp = 0;
                    uint32_t min_timestamp_interval = std::numeric_limits<uint32_t>::max();
                    uint32_t max_timestamp_interval = std::numeric_limits<uint32_t>::min();
                    double min_value = std::numeric_limits<double>::max();
                    double max_value = std::numeric_limits<double>::min();
                    double aggregated_value = 0;
                    std::vector<uint32_t> all_intervals;
                    for (klio::readings_it_t it = readings->begin();
                            it != readings->end(); it++) {
                        num_readings++;
                        klio::timestamp_t ts1 = (*it).first;
                        double val1 = (*it).second;
                        min_value = std::min(min_value, val1);
                        max_value = std::max(max_value, val1);
                        aggregated_value += val1;
                        if (last_timestamp == 0) {
                            // this is the first value we read, initialize state
                            first_timestamp = ts1;
                            last_timestamp = ts1;
                        } else {
                            // everything set up, this is just another value
                            uint32_t interval = ts1 - last_timestamp;
                            all_intervals.push_back(interval);
                            min_timestamp_interval = std::min(min_timestamp_interval, interval);
                            max_timestamp_interval = std::max(max_timestamp_interval, interval);
                            // Update state variables
                            last_timestamp = ts1;
                        }
                    }
                    size_t middle_element_idx = all_intervals.size() / 2;
                    uint32_t mean_interval = 0;
                    if (middle_element_idx != 0) {
                        std::nth_element(all_intervals.begin(),
                                all_intervals.begin() + middle_element_idx,
                                all_intervals.end());
                        mean_interval = all_intervals[middle_element_idx];
                    }
                    // compose output line
                    std::cout << std::setfill(' ') << std::setw(8) << num_readings;
                    std::cout << std::setw(8) << mean_interval;
                    // print mean of intervals
                    if (min_timestamp_interval == std::numeric_limits<uint32_t>::max() ||
                            max_timestamp_interval == std::numeric_limits<uint32_t>::min()) {
                        std::cout << std::setw(8) << "n/a";
                        std::cout << std::setw(8) << "n/a";
                    } else {
                        std::cout << std::setw(8) << min_timestamp_interval;
                        std::cout << std::setw(8) << max_timestamp_interval;
                    }
                    // value stats
                    if (num_readings == 0) {
                        std::cout << std::setw(8) << "n/a";
                    } else {
                        std::cout << std::setw(8) << (uint32_t) aggregated_value / num_readings;
                    }
                    std::cout << std::setw(8) << (uint32_t) min_value;
                    std::cout << std::setw(8) << (uint32_t) max_value;
                    // Time conversion foo
                    klio::LocalTime::Ptr lt(new klio::LocalTime("."));
                    boost::local_time::local_time_facet* output_facet
                            = new boost::local_time::local_time_facet();
                    output_facet->format("%Y.%m.%d-%H:%M:%S");
                    std::ostringstream oss;
                    oss.imbue(std::locale(std::locale::classic(), output_facet));
                    boost::local_time::local_date_time first_timestamp_datetime =
                            lt->get_local_time(loadedSensor, first_timestamp);
                    oss.str("");
                    oss << first_timestamp_datetime;
                    std::cout << std::setw(21) << oss.str();
                    boost::local_time::local_date_time last_timestamp_datetime =
                            lt->get_local_time(loadedSensor, last_timestamp);
                    oss.str("");
                    oss << last_timestamp_datetime;
                    std::cout << std::setw(21) << oss.str();
                    // name of the sensor
                    std::cout << '\t' << loadedSensor->name() << std::endl;
                }
            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

        /**
         * SYNC action
         */
        } else if (boost::iequals(action, std::string("sync"))) {

            std::string sourcestore;

            if (!vm.count("sourcestore")) {
                std::cerr << "You must specify a source store for synchronization." << std::endl;
                return 1;

            } else {
                sourcestore = vm["sourcestore"].as<std::string>();
            }

            bfs::path source_path(sourcestore);
            if (!bfs::exists(source_path)) {
                std::cerr << "File " << source_path << " does not exist, exiting." << std::endl;
                return 2;
            }
            bfs::path target_path(storefile);
            if (!bfs::exists(target_path)) {
                std::cerr << "File " << target_path << " does not exist, exiting." << std::endl;
                return 2;
            }

            klio::StoreFactory::Ptr factory(new klio::StoreFactory());
            try {

                std::cout << "Attempting to open source store " << source_path << std::endl;
                klio::Store::Ptr source_store(factory->open_sqlite3_store(source_path));
                std::cout << "Opened source store: " << source_store->str() << std::endl;

                std::cout << "Attempting to open target store " << target_path << std::endl;
                klio::Store::Ptr target_store(factory->open_sqlite3_store(target_path));
                std::cout << "Opened target store: " << target_store->str() << std::endl;

                std::cout << "Synchronizing stores..." << std::endl;
                target_store->sync(source_store);
                std::cout << "Stores Synchronized." << std::endl;

                source_store->close();
                target_store->close();

            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to synchronize stores. " << ex.what() << std::endl;
            }


        /** 
         * unknown command
         */
        } else {
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
