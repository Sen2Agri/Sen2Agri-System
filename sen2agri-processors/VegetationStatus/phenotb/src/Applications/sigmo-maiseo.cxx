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
#include "phenoFunctions.h"
#include "miscUtils.h"
#include <fstream>
#include <string>
#include <iostream>
#include <regex>

namespace pheno{
std::tuple<DateVector, std::vector<long int>, std::vector<VectorType> > get_csv_profiles(const std::string& fname)
{
  /* parcelle;mean_2013-02-16;mean_2013-02-17;mean_2013-02-21;mean_2013-02-22;mean_2013-02-27;mean_2013-03-03;mean_2013-03-04;mean_2013-03-08;mean_2013-03-14;mean_2013-03-18;mean_2013-03-19;mean_2013-03-23;mean_2013-03-24;mean_2013-03-29;mean_2013-04-03;mean_2013-04-12;mean_2013-04-13;mean_2013-04-14;mean_2013-04-17;mean_2013-04-22;mean_2013-04-23;mean_2013-05-17;mean_2013-05-23;mean_2013-05-27;mean_2013-06-06;mean_2013-06-07;mean_2013-06-11;mean_2013-06-12;mean_2013-06-16;mean_2013-07-19;mean_2013-08-04;mean_2013-08-20;mean_2013-09-05;mean_2013-10-07;mean_2013-10-23;mean_2013-12-10*/

  DateVector dates;
  std::ifstream csvFile(fname);
  if(!csvFile)
    itkGenericExceptionMacro(<< "Could not open file " << fname << "\n");

  std::string line;

  //parse the dates
  csvFile >> line;
  auto tokens = string_split(line, ";");
  auto tok_it = std::begin(tokens);
  ++tok_it;
  while(tok_it != std::end(tokens))
    {
    if(*tok_it=="") break;
    auto year = (*tok_it).substr(5,4);
    auto month = (*tok_it).substr(10,2);
    auto day = (*tok_it).substr(13,2);
    dates.push_back(make_date(year+month+day));
    ++tok_it;
    }
  std::vector<VectorType> res;
  std::vector<long int> ids;
  long int id_counter{0};
  while(csvFile >> line)
    {
    std::cout << line << "\n";
    auto ttokens = string_split(line, ";");
    auto ttok_it = std::begin(ttokens);
    try{
    ids.push_back(std::stoi(*ttok_it));
    }
    catch(const std::invalid_argument& e){
    ids.push_back(id_counter);
    }
    ++id_counter;
    ++ttok_it;
    VectorType profile(dates.size());
    std::size_t t{0};
    while(ttok_it != std::end(ttokens))
      {
      PrecisionType value{NAN};
      if((*ttok_it)!="")
        value = std::stod(*ttok_it);
      profile[t] = value;
      ++ttok_it;
      ++t;
      }
    res.push_back(profile);
    }
  return std::make_tuple(dates,ids,res);
}

}

int main(int argc, char* argv[])
{
  if(argc!=3)
    {
    std::cerr << "Usage: " << argv[0] << " filename output-prefix\n";
    exit(1);
    }
  auto parse_result = pheno::get_csv_profiles(argv[1]);
  auto dates = std::get<0>(parse_result);
  auto ids = std::get<1>(parse_result);
  auto profiles = std::get<2>(parse_result);
  std::vector<int> doys;
  for(auto d : dates)
    doys.push_back(pheno::doy(d));
  std::cout << "There are " << dates.size() << " dates and "
            << profiles.size() << " profiles.\n";

  for(size_t pfid=0; pfid<profiles.size(); ++pfid)
    {
    std::cout << pfid << " ... ";
    std::cout.flush();
    auto vec = profiles[pfid];
    auto pred = [=](int e){return !(std::isnan(vec[e]));};
    auto f_profiles = pheno::filter_profile(vec, dates, pred);
    decltype(vec) profile=f_profiles.first;
    decltype(vec) t=f_profiles.second;
    if(profile.size()>5)
      {
      auto approx = pheno::normalized_sigmoid::TwoCycleApproximation(profile, t);
      auto principal_cycle = std::get<1>(approx);
      auto secondary_cycle = std::get<2>(approx);
      auto x = std::get<0>(principal_cycle);
      auto yHat = std::get<0>(approx);
      auto residuals = std::get<3>(principal_cycle);
      auto err = std::get<4>(principal_cycle);

      std::stringstream fnss;
      fnss << argv[2] << ids[pfid]  << ".gp";

      std::stringstream title;
      title <<   ids[pfid] << "  : ";

      for(auto & elem : x)
        {
        title << elem << " ";
        }
      title << "| e=" << err;

      auto ed = pheno::normalized_sigmoid::error_diagnostics(x, profile, t);
      switch(ed) {
      case pheno::FittingErrors::OK:
        title << "| ED: OK";
        break;
      case pheno::FittingErrors::NOK:
        title << "| ED: NOK";
        break;
      case pheno::FittingErrors::HasNan:
        title << "| ED: NaN";
        break;
      }

      pheno::generatePlotFile(fnss.str(), title.str(), t, profile, yHat, x);
      }
    }
  std::cout << std::endl;
  return 0;
}
