#ifndef LIBKLIO_ALGORITHM_COLLATE_HPP
#define LIBKLIO_ALGORITHM_COLLATE_HPP 1

#include <libklio/common.hpp>
#include <libklio/types.hpp>
#include <libklio/store.hpp>
#include <vector>

namespace klio {
 // template <typename Array>
 //   void render_sensordata_array(std::ostream& os, const Array& A);
 // template<> void klio::render_sensordata_array<double>
 //     (std::ostream& os, const double& x);

  klio::sensordata_table_t collate(Store::Ptr store, const sensors_t& sensors);    

  std::vector<double> get_sensordata_row(
      const klio::sensordata_table_t& table, size_t row_index);

  // template implementation below
  template <typename Array>
    void render_sensordata_array(std::ostream& os, const Array& A)
    {
      typename Array::const_iterator i;
      os << "[";
      for (i = A.begin(); i != A.end(); ++i) {
        render_sensordata_array(os, *i);
        if (boost::next(i) != A.end())
          os << ',';
      }
      os << "]" << std::endl;
    };

  template<> 
    inline void render_sensordata_array<double>(std::ostream& os, const double& x)
    {
      os << x;
    };

};


#endif /* LIBKLIO_ALGORITHM_COLLATE_HPP */

