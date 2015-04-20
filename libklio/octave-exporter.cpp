#include <vector>
#include <boost/algorithm/string.hpp>
#include <libklio/octave-exporter.hpp>


using namespace klio;

static const uint32_t SECONDS_PER_DAY = 60 * 60 * 24;
static const uint32_t MINUTES_PER_DAY = 60 * 24;

void OctaveExporter::write_lead_in(const std::string& name,
        const std::string& description) {
    _out << "% script file containing sensor data of sensor "
            << name << ", " << description << std::endl;
    _out << "1;" << std::endl;
}

static uint32_t calc_next_day_timestamp(uint32_t ts) {
    // Use this formula to determine the beginning of the next day:
    // $nextday = $current_timestamp + 86400 - ($current_timestamp % 86400);
    return (ts + SECONDS_PER_DAY - (ts % SECONDS_PER_DAY));
}

void OctaveExporter::write_values_function(const std::string& name,
        klio::readings_t_Ptr readings) {
    klio::readings_it_t it;

    _out << "function values = get_" << name << "_values()" << std::endl;
    _out << "  values = [ ..." << std::endl << "\t";

    std::vector<double> current_day(MINUTES_PER_DAY);
    std::vector<double>::iterator day_it;
    int32_t next_day_timestamp = 0;

    // Look for the first reading and determine the first timestamp in the dataset.
    it = readings->begin();
    klio::timestamp_t ts = (*it).first;
    next_day_timestamp = calc_next_day_timestamp(ts);

    for (it = readings->begin(); it != readings->end(); it++) {
        klio::timestamp_t ts = (*it).first;
        double val = (*it).second;
        //std::cout << ts << "\t" << val << std::endl;
        if (ts >= next_day_timestamp) {
            //    std::cout << " TS: " << ts << " next_day: " << next_day_timestamp << std::endl;
            // write out the line
            for (uint16_t i = 0; i < MINUTES_PER_DAY; i++) {
                _out << current_day[i];
                if (i < MINUTES_PER_DAY - 1)
                    _out << ", ";
                else
                    _out << "; ..." << std::endl << "\t";
            }
            // clear the datastructures, setup for the next day
            next_day_timestamp = calc_next_day_timestamp(ts + 1);
            current_day.clear();
            current_day.resize(MINUTES_PER_DAY);
        }
        // Calculate position of this value within the day
        std::vector<double>::size_type position =
                (ts - (next_day_timestamp - SECONDS_PER_DAY)) / 60;
        // Check for duplicate values - avoid collisions.
        if (current_day[position] != 0) {
            if (position < MINUTES_PER_DAY)
                current_day[position + 1] = val;
        } else {
            current_day[position] = val;
        }
        //  std::cout << position << " / " << val << std::endl;
    }
    // Finish the last line - there is a difference: this one is not
    // terminated by a ";" symbol.
    for (uint16_t i = 0; i < MINUTES_PER_DAY; i++) {
        _out << current_day[i];
        if (i < MINUTES_PER_DAY - 1)
            _out << ", ";
        else
            _out << " ..." << std::endl;
    }
    _out << "  ];" << std::endl;
    _out << "end" << std::endl;
}

void OctaveExporter::write_description_function(const std::string& description,
        const std::string& name) {
    _out << "function desc = get_" << name << "_description()" << std::endl;
    _out << "  desc = '" << description << "';" << std::endl;
    _out << "end" << std::endl;
}

void OctaveExporter::process(klio::readings_t_Ptr readings,
        const std::string& name, const std::string& description) {
    write_lead_in(name, description);
    std::string clean_name =
            boost::to_lower_copy(
            boost::ireplace_all_copy(
            boost::ireplace_all_copy(
            boost::ireplace_all_copy(name,
            "-", "_"),
            "%", "_"),
            ":", "")
            );
    write_description_function(clean_name, description);
    write_values_function(clean_name, readings);
}
