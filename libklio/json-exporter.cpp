#include <libklio/json-exporter.hpp>


using namespace klio;

void JSONExporter::process(klio::readings_t_Ptr readings,
        const std::string& name, const std::string& description) {
    _out << "[";
    klio::readings_it_t it;
    for (it = readings->begin(); it != readings->end(); ++it) {
        klio::timestamp_t ts1 = (*it).first;
        double val1 = (*it).second;
        _out << "[" << ts1 << "," << val1 << "]";
        if (it != (--readings->end()))
            _out << ",";
    }

    _out << "]" << std::endl;
}
