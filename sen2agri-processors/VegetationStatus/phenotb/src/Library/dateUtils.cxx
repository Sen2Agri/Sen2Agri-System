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

#include "dateUtils.h"
#include "miscUtils.h"
#include "itkMacro.h"
#include <boost/regex.hpp>
#include <fstream>
#include <cmath>
#include <algorithm>

namespace pheno {
std::tm make_date(const std::string& d)
{
    std::tm dd;
    dd.tm_hour = dd.tm_min = dd.tm_sec = 0;
    dd.tm_year = std::stoi(d.substr(0, 4));
    dd.tm_mon = std::stoi(d.substr(4, 2))-1;
    dd.tm_mday = std::stoi(d.substr(6, 2));
    return dd;
}

int delta_days(std::tm t1, std::tm t2)
{
return std::round(difftime(mktime(&t1),mktime(&t2))/double{60*60*24});
}

unsigned int doy(const std::tm& d)
{
  auto jan1st = d;
  jan1st.tm_year = d.tm_year;
  jan1st.tm_mon = 0;          /* month of year (0 - 11) */
  jan1st.tm_mday = 1;
  return delta_days(d,jan1st)+1;
}

void reduce_to_first_year(std::vector<std::time_t> &times)
{
  auto end = std::end(times);
  auto it = std::min_element(std::begin(times), end);
  if (it == end)
    return;

  std::tm tm, tm2 = { };
  localtime_r(&*it, &tm);

  tm2.tm_year = tm.tm_year;
  tm2.tm_mon = 0;
  tm2.tm_mday = 1;

  auto yearStart = std::mktime(&tm2);
  std::transform(std::begin(times), std::end(times), std::begin(times),
                 [yearStart](std::time_t time) { return time - yearStart; });
}

std::vector<int> tm_to_doy_list(const std::vector<std::tm> &times)
{
  std::vector<int> res;
  std::vector<time_t> seconds;
  if (times.empty())
    return res;

  seconds.resize(times.size());
  res.resize(times.size());

  std::transform(std::begin(times), std::end(times), std::begin(seconds),
                 [](std::tm tm) { return std::mktime(&tm); });

  reduce_to_first_year(seconds);

  constexpr int secondsPerDay = 60 * 60 * 24;
  constexpr float secondsPerDayF = static_cast<float>(secondsPerDay);

  std::transform(std::begin(seconds), std::end(seconds), std::begin(res),
                 [](std::time_t time) { return static_cast<int>(lrintf(time / secondsPerDayF)+1); });

  return res;
}

int months(const std::string& m)
{
  static std::map<std::string, int> mm {
    {"jan", 1}, {"feb", 2}, {"mar", 3}, {"apr", 4}, {"may", 5}, {"jun", 6}, {"jul", 7}, {"aug", 8}, {"sep", 9}, {"oct", 10}, {"nov", 11}, {"dec", 12}
  };
  return mm[m];
}

DateVector parse_date_file(const std::string& df)
{
  std::ifstream dateFile(df);
  if(!dateFile)
    itkGenericExceptionMacro(<< "Could not open file " << df << "\n");
  DateVector vdates;
  std::string date{""};
  while(dateFile>>date)
    {
    auto dd(make_date(date));
    vdates.push_back(dd);
    }
  return vdates;
}

std::pair<DateVector, int> parse_dates_csv_file(const std::string& df, int year)
{
  std::ifstream csvFile(df);
  if(!csvFile)
    itkGenericExceptionMacro(<< "Could not open file " << df << "\n");

  std::string header;
  csvFile >> header;

  boost::regex e {R"((\d{1,2})-(\w{3}))"};

  DateVector vdates;

  int pos{0};
  bool found_date{};  
  for(auto t : string_split(header, ","))
    {
    boost::smatch matches;
    if (boost::regex_match(t,matches,e))
      {
      found_date = true;
      std::string sday{pad_int(std::stoi(matches[1]))};
      std::string month{matches[2].str()};
      std::transform(std::begin(month),std::end(month),std::begin(month),tolower);
      auto dd = make_date(std::to_string(year)
                                 +pad_int(months(month))
                                 +sday);
      vdates.push_back(dd);
      }
    if(!found_date) pos++;
    }

  return {vdates, pos};
}

std::vector<VectorType> get_csv_profiles(const std::string& fname, int pos, int ndates)
{
  std::vector<VectorType> res;

  std::ifstream csvFile(fname);
  if(!csvFile)
    itkGenericExceptionMacro(<< "Could not open file " << fname << "\n");

  std::string line;
  csvFile >> line;

  while(csvFile >> line)
    {
    auto tokens = string_split(line, ",");
    VectorType profile(ndates);
    for(auto i=pos; i<pos+ndates; i++)
      {
        profile[i-pos] = std::stod(tokens[i]);
      }
    res.push_back(profile);
    }
  return res;
}

DateVector parse_dates_sirhyus(const std::string& fname)
{
  std::ifstream csvFile(fname);
  if(!csvFile)
    itkGenericExceptionMacro(<< "Could not open file " << fname << "\n");

  std::string header;
  csvFile >> header;

  DateVector res;

  auto fields = string_split(header, ";");

  std::for_each(std::begin(fields)+1, std::end(fields), [&res](std::string e)
          {
          res.push_back(make_date("20"+e.substr(1,7)));
          });

  return res;

}

std::vector<std::pair<long int, VectorType>> get_sirhyus_profiles(const std::string& df, unsigned int nbDates)
{
  std::vector<std::pair<long int, VectorType>> res;

  std::ifstream csvFile(df);
  if(!csvFile)
    itkGenericExceptionMacro(<< "Could not open file " << df << "\n");

  std::string line;

  //skip the header line
  csvFile >> line;

  while(csvFile >> line)
    {
    auto tokens = string_split(line, ";");
    long int pid = std::stoi(tokens[0]);
    if(nbDates<tokens.size()-1)
      itkGenericExceptionMacro(<< "More measures than expected dates!" << tokens.size()-1 << "/" << nbDates << "\n");  
    VectorType profile(nbDates, double{0});
    int count{0};
    std::for_each(std::begin(tokens)+1, std::end(tokens),[&profile,&count](std::string e)
                  {
                  double val{(e=="")?NAN:std::stod(e)};
                  profile[count++]=val;
                  });
    res.push_back({pid, profile});

    }
  return res;
}
}
