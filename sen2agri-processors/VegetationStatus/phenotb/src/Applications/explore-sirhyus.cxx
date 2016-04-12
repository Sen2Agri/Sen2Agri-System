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
#include <string>
#include <iostream>

namespace pheno{
namespace normalized_sigmoid{

}
}

int main(int argc, char* argv[])
{
  if(argc!=2 && argc!=3)
    {
    std::cerr << "Usage: " << argv[0] << " filename [profile_idx]\n";
    exit(1);
    }

  auto dates = pheno::parse_dates_sirhyus(argv[1]);

  auto year = dates[0].tm_year;

  std::vector<int> doys;
  for(auto d : dates)
    {
    doys.push_back(pheno::doy(d));
    }
  
  std::cout << "There are " << dates.size() << " dates ";

  auto data = pheno::get_sirhyus_profiles(argv[1], dates.size());

  std::cout << "and " << data.size() << " profiles.\n";


  unsigned int pidx{0};
  auto last_profile = data.size()-1;
  if(argc==3)
    {
    pidx=std::stoi(argv[2]);
    last_profile=pidx+1;
    }

  int wrong{0};
  while(pidx!=last_profile)
    {

    std::cout << "---------------------------------------\n";
    std::cout <<  "Processing profile " << data[pidx].first << ".\n";
    auto vec = data[pidx].second;

    if(vec.size()!=dates.size())
      {
      wrong++;
      pidx++;
      continue;
      }
    
    for(auto v : vec)
      std::cout << v << std::endl;
    
    auto pred = [=](int e){return !(std::isnan(vec[e]));};

    auto f_profiles = pheno::filter_profile(vec, dates, pred);

    decltype(vec) profile=f_profiles.first;
    decltype(vec) t=f_profiles.second;

    std::cout << t.size() << " dates are valid.\n";
  
    for(size_t i=0; i<profile.size(); i++)
      std::cout << t[i] << " " << profile[i] << std::endl;

    std::cout << std::endl;

    auto approx = pheno::normalized_sigmoid::TwoCycleApproximation(profile, t);
    auto principal_cycle = std::get<1>(approx);
    auto secondary_cycle = std::get<2>(approx);
    auto x = std::get<0>(principal_cycle);
    auto yHat = std::get<0>(approx);
    auto residuals = std::get<3>(principal_cycle);
    auto err = std::get<4>(principal_cycle);
    
    std::stringstream fnss;
    fnss << "/tmp/sirhyus-" << year << "-" << data[pidx].first  << ".gp";

    std::stringstream title;
    title <<   data[pidx].first << "-" << year << "  : ";

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
    ++pidx;
    }

  std::cout << wrong << "/" << data.size() << std::endl;
  return 0;
}
