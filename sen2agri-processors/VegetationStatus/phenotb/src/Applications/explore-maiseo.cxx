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

}

int main(int argc, char* argv[])
{
  if(argc!=3 && argc!=4)
    {
    std::cerr << "Usage: " << argv[0] << " filename year [profile_idx]\n";
    exit(1);
    }

  auto year = std::stoi(argv[2]);
  auto parse_result = pheno::parse_dates_csv_file(argv[1], year);
  auto dates = parse_result.first;

  std::cout << "Dates start at position " << parse_result.second << std::endl;
  std::vector<int> doys;
  for(auto d : dates)
    doys.push_back(pheno::doy(d));
  
  std::cout << "There are " << dates.size() << " dates ";

  auto profiles = pheno::get_csv_profiles(argv[1], parse_result.second, dates.size());
  bool done{false};
  int pfid{0};
  while(!done)
    {
    if(argc==4)
      {
      pfid = {std::stoi(argv[3])};
      if(pfid>=static_cast<int>(profiles.size()) || pfid<0)
        {
        std::cerr << "\nProfile index out of range. There are only " << profiles.size()+1 << " profiles.\n";
        exit(1);
        }
      done = true;
      }
  
    auto vec = profiles[pfid];
    auto pred = [=](int e){return !(std::isnan(vec[e]));};

    auto f_profiles = pheno::filter_profile(vec, dates, pred);

    decltype(vec) profile=f_profiles.first;
    decltype(vec) t=f_profiles.second;

    std::cout << " and " << t.size() << " are valid. Processing " << pfid << "\n";
  
    for(size_t i=0; i<profile.size(); i++)
      std::cout << t[i] << " " << profile[i] << std::endl;

    std::cout << std::endl;

    auto minmax = std::minmax_element(std::begin(profile), std::end(profile));
  
    auto vmax = *(minmax.second);
    auto vmin = *(minmax.first);
      
    profile = (profile-vmin)/(vmax-vmin);

    auto pradius = 3;
    auto pmax = minmax.second - std::begin(profile);
    if(pmax < 4)
      pradius = 2;

    pheno::FilterType<typename pheno::VectorType> lp = pheno::getLocalParabola<typename pheno::VectorType>;
    auto x(pheno::normalized_sigmoid::guesstimator(
             pheno::profileFilter(
               profile,
               t, lp, pradius),
             t));
    std::cout << x << std::endl;

    auto fprofile = pheno::gaussianWeighting(profile, t);
    
    pheno::normalized_sigmoid::F<pheno::VectorType> phFs;
    pheno::ParameterCostFunction<pheno::normalized_sigmoid::F<pheno::VectorType>> fs{4, t.size(), fprofile, t, phFs};
    auto err(pheno::optimize(x, fs));
    std::cout << x << std::endl;  

    pheno::VectorType yHat(t.size());
    phFs(t,x,yHat);

    yHat = yHat*(vmax-vmin)+vmin;
    profile = profile*(vmax-vmin)+vmin;

    std::stringstream fnss;
    fnss << "/tmp/maiseo-" << pfid  << ".gp";

    std::stringstream title;
    title <<  pfid << "-" << year << "  : ";

    for(auto & elem : x)
      {
      title << elem << " ";
      }
    title << "| e=" << err;

    
    pheno::generatePlotFile(fnss.str(), title.str(), t, profile, yHat, x);

    if(pfid==static_cast<int>(profiles.size()-1)) done = true;
    else pfid++;
    }
  return 0;
}
