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
#include "itkMacro.h"
#include "phenoFunctions.h"
#include <vector>
#include <algorithm>


template <typename T>
inline
bool err(T x, T y)
{
  if (fabs(x-y)>10e-5) return true;
  return false;
}

int phenoNormalizedSigmoidMetrics(int argc, char * argv[])
{
  if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  std::vector<std::pair<std::tuple<double, double, double, double, double, double>,
                        std::tuple<double, double, double, double, double, double>>> 
    tdvm;
  tdvm.push_back(
    std::make_pair(
      std::make_tuple(75.0, 7.0, 250.0, 10.0, 8.0, 0.0), //x0, x1, x2, x3, max, min
      std::make_tuple(0.0357143, 61.0, 89.0, 230.0, 270.0, -0.025) //dgx0, t0, t1, t2, t3, dgx2
      ));
    
  for(auto td : tdvm)
    {
    auto xvals = td.first;
    auto expected = td.second;
    std::vector<double> x{std::get<0>(xvals), std::get<1>(xvals),
        std::get<2>(xvals), std::get<3>(xvals)};
    double dgx0, t0, t1, t2, t3, dgx2;
    std::tie(dgx0, t0, t1, t2, t3, dgx2) = 
      pheno::normalized_sigmoid::pheno_metrics<double>(x);

    if(err(dgx0,std::get<0>(expected)) ||
       err(t0,std::get<1>(expected)) ||
       err(t1,std::get<2>(expected)) ||
       err(t2,std::get<3>(expected)) ||
       err(t3,std::get<4>(expected)) ||
       err(dgx2,std::get<5>(expected)))
      {
      std::cout << dgx0 << " : " << std::get<0>(expected)
                << ", " 
                <<  t0 << " : " << std::get<1>(expected)
                << ", " 
                <<  t1 << " : " << std::get<2>(expected)
                << ", " 
                <<  t2 << " : " << std::get<3>(expected)
                << ", " 
                <<  t3 << " : " << std::get<4>(expected)
                << ", " 
                <<  dgx2 << " : " << std::get<5>(expected)
                << std::endl;
      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}
