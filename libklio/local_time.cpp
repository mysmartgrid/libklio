#include "local_time.hpp"
#include <boost/filesystem.hpp>
namespace bfs = boost::filesystem; 

using namespace klio;
using namespace boost::gregorian; 
using namespace boost::local_time;
using namespace boost::posix_time;



LocalTime::LocalTime(const char* cmd) 
  : _tz_db()
{
  try {
    bfs::path zonespec_filename("date_time_zonespec.csv");
    std::ifstream zs_stream;
    if (bfs::exists(bfs::path(cmd) / ".." / zonespec_filename)) {
      _tz_db.load_from_file(
          (bfs::path("/usr/share/libklio") / zonespec_filename).c_str()
        );
    } else if (bfs::exists(bfs::path("./share/libklio") / zonespec_filename)) {
      _tz_db.load_from_file(
          (bfs::path("./share/libklio") / zonespec_filename).c_str()
        );
    } else {
      std::cerr << "Cannot open " << zonespec_filename << ", aborting." << std::endl;
      exit(-1);
    }
  }catch(data_not_accessible dna) {
    std::cerr << "Error with time zone data file: " << dna.what() << std::endl;
    exit(EXIT_FAILURE);
  }catch(bad_field_count bfc) {
    std::cerr << "Error with time zone data file: " << bfc.what() << std::endl;
    exit(EXIT_FAILURE);
  }
}

time_zone_ptr LocalTime::get_timezone_ptr(klio::Sensor::Ptr sensor) {
  return _tz_db.time_zone_from_region(sensor->timezone());
}

boost::local_time::local_date_time
  LocalTime::get_local_time(klio::Sensor::Ptr sensor, klio::timestamp_t time) 
{
  time_zone_ptr tz(get_timezone_ptr(sensor));
  ptime pt = boost::posix_time::from_time_t(time);
  return local_date_time(pt, tz);
}

klio::timestamp_t 
  LocalTime::get_timestamp(boost::posix_time::ptime ptime) 
{
  static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));
  time_duration diff(ptime - epoch);
  return (diff.ticks() / diff.ticks_per_second());
}


klio::timestamp_t 
  LocalTime::get_timestamp(klio::Sensor::Ptr sensor, boost::local_time::local_date_time time)
{
  static boost::posix_time::ptime epoch(boost::gregorian::date(1970, 1, 1));

  time_duration diff = time.utc_time() - epoch;
  return diff.total_seconds();
}
