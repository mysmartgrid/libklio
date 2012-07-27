#ifndef LIBKLIO_LOCAL_TIME_HPP
#define LIBKLIO_LOCAL_TIME_HPP 1

#include <libklio/common.hpp>
#include <libklio/sensor.hpp>
#include <libklio/types.hpp>
#include <boost/date_time/local_time/local_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

namespace klio {
  class LocalTime {
    public:
      typedef std::tr1::shared_ptr<LocalTime> Ptr;
      LocalTime (const char* cmd);
      virtual ~LocalTime() {};

      std::vector<std::string> get_valid_timezones();
      bool is_valid_timezone(const std::string& zone);

      boost::local_time::time_zone_ptr 
        get_timezone_ptr(klio::Sensor::Ptr sensor);

      boost::local_time::local_date_time
        get_local_time(klio::Sensor::Ptr sensor, klio::timestamp_t time);

      boost::posix_time::ptime
        get_utc_time(klio::Sensor::Ptr sensor, klio::timestamp_t time);

      boost::gregorian::date
        get_local_date(klio::Sensor::Ptr sensor, klio::timestamp_t time);

      uint16_t
        get_local_day_of_year(klio::Sensor::Ptr sensor, klio::timestamp_t time);
      uint16_t
        get_local_hour(klio::Sensor::Ptr sensor, klio::timestamp_t time);

      klio::timestamp_t 
        get_timestamp(boost::posix_time::ptime ptime);

      klio::timestamp_t 
        get_timestamp(klio::Sensor::Ptr sensor, boost::local_time::local_date_time time);

    private:
      LocalTime (const LocalTime& original);
      LocalTime& operator= (const LocalTime& rhs);
      boost::local_time::tz_database _tz_db;
      
  };
};


#endif /* LIBKLIO_LOCAL_TIME_HPP */

