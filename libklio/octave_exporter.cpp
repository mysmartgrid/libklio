#include "octave_exporter.hpp"
#include <boost/algorithm/string.hpp>
using namespace boost;
using namespace klio;

static const uint16_t SECONDS_PER_DAY=60*60*24;

void OctaveExporter::write_lead_in(const std::string& name,
    const std::string& description) {
  _out << "% script file containing sensor data of sensor " 
    << name << ", " << description << std::endl;
  _out << "1;" << std::endl;
}

void OctaveExporter::write_values_function(const std::string& name,
    klio::readings_t_Ptr readings) {
  klio::readings_it_t it;

  _out << "function values = get_" << name << "_values()" << std::endl;
  _out << "  values = [ ..." << std::endl;

  // See http://www.cplusplus.com/reference/algorithm/min_element/
  // $nextday = $current_timestamp + 86400 - ($current_timestamp % 86400);

  // 1. determine position of first element in the day

  // 2. Now, we can write out every value sequentially


  for(  it = readings->begin(); it != readings->end(); it++) {
    klio::timestamp_t ts1=(*it).first;
    double val1=(*it).second;
    std::cout << ts1 << "\t" << val1 << std::endl;
  }

  

  _out << "  ]" << std::endl;
  _out << "end" << std::endl;
}

void OctaveExporter::write_description_function(const std::string& description,
    const std::string& name ) {
  _out << "function desc = get_" << name << "_description()" << std::endl;
  _out << "  desc = '" << description << "';" << std::endl;
  _out << "end" << std::endl;
}

void OctaveExporter::process(klio::readings_t_Ptr readings,
    const std::string& name, const std::string& description) {
  write_lead_in(name, description);
  std::string clean_name=
    to_lower_copy( ireplace_all_copy( name, ":", ""));
  write_description_function(clean_name, description);
  write_values_function(clean_name, readings);
}

