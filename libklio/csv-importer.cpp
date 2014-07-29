#include<fstream>
#include <stdlib.h>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <libklio/time.hpp>
#include "csv-importer.hpp"


using namespace klio;

readings_t_Ptr CSVImporter::process() {

    readings_t_Ptr readings(new readings_t());
    const TimeConverter::Ptr time_converter(new TimeConverter());
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;

    std::vector<std::string> record;
    std::string line;

    std::cout << "Importing readings using separator: " << _separator << std::endl;
    boost::char_separator<char> separator(_separator.c_str());

    while (getline(_in, line)) {

        if (!line.empty()) {

            Tokenizer tokenizer(line, separator);

            if (tokenizer.begin() != tokenizer.end()) {

                record.assign(tokenizer.begin(), tokenizer.end());

                if (record.size() > 1) {

                    try {
                        timestamp_t timestamp = boost::lexical_cast<timestamp_t>(record.at(0));
                        double reading = boost::lexical_cast<double>(record.at(1));
                        std::cout << timestamp << ": " << reading << std::endl;

                        readings->insert(std::pair<timestamp_t, double>(
                                time_converter->convert_from_epoch(atoi(record.at(0).c_str())),
                                reading));

                    } catch (boost::bad_lexical_cast &) {
                        //Ignore
                    }
                }
            }
        }
    }
    return readings;
}
