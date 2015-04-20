/**
 * This file is part of libklio.
 *
 * (c) Fraunhofer ITWM - Mathias Dalheimer <dalheimer@itwm.fhg.de>,    2010
 *                       Ely de Oliveira   <ely.oliveira@itwm.fhg.de>, 2013
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
 */

#ifndef LIBKLIO_ALGORITHM_COLLATE_HPP
#define LIBKLIO_ALGORITHM_COLLATE_HPP 1

#include <libklio/store.hpp>


namespace klio {
    // template <typename Array>
    //   void render_sensordata_array(std::ostream& os, const Array& A);
    // template<> void klio::render_sensordata_array<double>
    //     (std::ostream& os, const double& x);

    klio::sensordata_table_t collate(Store::Ptr store, const sensors_t& sensors);

    std::vector<double> get_sensordata_row(
            const klio::sensordata_table_t& table, unsigned long int row_index);

    // template implementation below

    template <typename Array>
    void render_sensordata_array(std::ostream& os, const Array& A) {
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
    inline void render_sensordata_array<double>(std::ostream& os, const double& x) {
        os << x;
    };

};

#endif /* LIBKLIO_ALGORITHM_COLLATE_HPP */
