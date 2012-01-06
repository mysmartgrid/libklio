#include "octave_exporter.hpp"
#include <boost/algorithm/string.hpp>
using namespace boost;
using namespace klio;

void OctaveExporter::write_lead_in(const std::string& name,
    const std::string& description) {
  _out << "% script file containing sensor data of sensor " 
    << name << ", " << description << std::endl;
  _out << "1;" << std::endl;
}

void OctaveExporter::write_description_function(const std::string& name,
    const std::string& description) {
  std::string clean_name=
    to_lower_copy( ireplace_all_copy( name, ":", ""));
  _out << "function desc = get_" << clean_name << "_description()" << std::endl;
  _out << "  desc = '" << description << "';" << std::endl;
  _out << "end" << std::endl;
}

void OctaveExporter::process(klio::readings_t_Ptr readings,
    const std::string& name, const std::string& description) {
  write_lead_in(name, description);
  write_description_function(name, description);
  //            klio::readings_it_t it;
  //            std::cout << "timestamp\treading" << std::endl;
  //            for(  it = readings->begin(); it != readings->end(); it++) {
  //              klio::timestamp_t ts1=(*it).first;
  //              double val1=(*it).second;
  //              std::cout << ts1 << "\t" << val1 << std::endl;
  //            }

}

