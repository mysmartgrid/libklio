#include<fstream>
#include <stdlib.h>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <libklio/time.hpp>
#include <libklio/csv-importer.hpp>


using namespace klio;

readings_t_Ptr CSVImporter::process() {

    readings_t_Ptr readings(new readings_t());
    const TimeConverter::Ptr time_converter(new TimeConverter());
    typedef boost::tokenizer<boost::char_separator<char> > Tokenizer;

    std::vector<std::string> record;
    std::string line;

    std::cout << "Importing readings using separators: \"" << _separators << "\"" << std::endl;
    boost::char_separator<char> separators(_separators.c_str());

    while (getline(_in, line)) {

        if (!line.empty()) {

            Tokenizer tokenizer(line, separators);

            if (tokenizer.begin() != tokenizer.end()) {

                record.assign(tokenizer.begin(), tokenizer.end());

                if (record.size() > 1) {

                    std::string column2 = record.at(1);
                    std::transform(column2.begin(), column2.end(), column2.begin(), ::tolower);

                    if (column2.find("nan") == std::string::npos) {

                        size_t pos = column2.find(',');
                        if (pos != std::string::npos) {
                            column2.replace(pos, 1, ".");
                        }

                        try {

                            timestamp_t timestamp = boost::lexical_cast<timestamp_t>(record.at(0));
                            double reading = boost::lexical_cast<double>(column2);
                            std::cout << timestamp << ": " << reading << std::endl;

                            readings->insert(std::pair<timestamp_t, double>(
                                    time_converter->convert_from_epoch(timestamp),
                                    reading));

                        } catch (boost::bad_lexical_cast &) {
                            //Ignore
                        }
                    }
                }
            }
        }
    }
    return readings;
}
