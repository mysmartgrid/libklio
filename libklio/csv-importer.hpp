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

#ifndef LIBKLIO_CSV_IMPORTER_HPP
#define LIBKLIO_CSV_IMPORTER_HPP 1

#include <libklio/importer.hpp>


namespace klio {

    class CSVImporter : public Importer {
    public:
        typedef boost::shared_ptr<CSVImporter> Ptr;

        CSVImporter(std::ifstream& in, std::string& separators) :
        Importer(in, separators) {
        };

        virtual ~CSVImporter() {
        };

        readings_t_Ptr process();

    private:
        CSVImporter(const CSVImporter& original);
        CSVImporter& operator=(const CSVImporter& rhs);
    };
}

#endif /* LIBKLIO_CSV_IMPORTER_HPP */
