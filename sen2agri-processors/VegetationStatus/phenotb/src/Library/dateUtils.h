/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
/*=========================================================================

  Program:   phenotb
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See phenotb-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef _DATEUTILS_H_
#define _DATEUTILS_H_


#include <string>
#include <map>
#include <vector>
#include "phenoTypes.h"

namespace pheno {
/// Makes a date (std::tm struct) from a string with format YYYYMMDD
std::tm make_date(const std::string& d);

/// Returns the difference in days between 2 time structures
int delta_days(std::tm t1, std::tm t2);

/// Return the day of year
unsigned int doy(const std::tm& d);

/** Makes all times relative to the start of the first year in the sequence
*/
void reduce_to_first_year(std::vector<std::time_t> &times);

/** Converts a sequence of struct tm to a list of time differences from the
    start of the first year in the sequence
*/
std::vector<int> tm_to_doy_list(const std::vector<std::tm> &times);

/** Parses an ASCII file containing a date per line (string with format YYYYMMDD)
    and returns an std::vector< std::tm > containing the dates in the order of reading
*/
DateVector parse_date_file(const std::string& df);

/** Parses the first line of a csv file looking for dates like 4-Feb
    and returns an std::vector< std::tm >. The year is passed as parameter. Returns a
    pair containing a vector of dates and the starting position (column) of the dates
    in the file.
*/
std::pair<DateVector, int> parse_dates_csv_file(const std::string& df, int year);

/** Extracts the profiles from a csv file (MAISEO format). Returns a vector of profiles.
    Gets as parameters the file name, the starting position of the dates (column) and the
    number of availble dates in the file.**/
std::vector<VectorType> get_csv_profiles(const std::string& fname, int pos, int ndates);

/** Parses the first line of a cvs file with this format:
    parcelle;f090215;f090317;f090321;f090603;f090623;f090701;f090712;f090726;f090805;f090814;f090830;f090906;f090924;f090930;f091016;f091225
    and returns a DateVector
*/
DateVector parse_dates_sirhyus(const std::string& fname);

/** Extracts the profiles from a csv file (Sirhyus format). Returns a vector of pairs
    where each pair is <field id, profile>
    Gets as parameters the file name.**/
std::vector<std::pair<long int, VectorType>> get_sirhyus_profiles(const std::string& df, unsigned int nbDates);
/// Map to convert months from text to int
int months(const std::string& m);
}

#endif
