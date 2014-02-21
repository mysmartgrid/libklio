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
                ("action,a", po::value<std::string>(), "Valid actions are: create, sync")
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
                store->open();
                store->initialize();
                std::cout << "Initialized store: " << store->str() << std::endl;
            } catch (klio::StoreException const& ex) {
                std::cout << "Failed to create: " << ex.what() << std::endl;
            }

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
