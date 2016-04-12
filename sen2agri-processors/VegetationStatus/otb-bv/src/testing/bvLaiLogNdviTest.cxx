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
  Program:   otb-bv
  Language:  C++

  Copyright (c) CESBIO. All rights reserved.

  See otb-bv-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkMacro.h"

#include "phenoFunctions.h"
#include <random>

using VectorType = pheno::VectorType;

struct lai_log_ndvi
{
    void operator()(const VectorType& ndvi, const VectorType& pars, VectorType &lai) const
    {
      auto k_lai = pars[0];
      auto ndvi_inf = pars[1];
      auto ndvi_soil = pars[2];
      for(auto i=0; i<3; ++i)
        lai[i] = -1.0/k_lai*log((ndvi[i]-ndvi_inf)/(ndvi_soil-ndvi_inf));
    }
};

int bvLaiLogNdvi(int argc, char * argv[])
{
  if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  unsigned int params{3};
  unsigned int nb_measures{100};
  VectorType lai_vec(nb_measures);
  VectorType ndvi_vec(nb_measures);

  VectorType x_test(params);
  x_test[0] = 2.0;
  x_test[1] = 0.9;
  x_test[2] = 0.1;
  auto rng = std::mt19937(std::random_device{}());
  rng.seed(5);
  std::normal_distribution<> d(0.0,0.01);
  for(size_t i=0; i<nb_measures; ++i)
    {
    ndvi_vec[i] = static_cast<double>(i)/nb_measures*0.8;
    lai_vec[i] = -1.0/x_test[0]*log((ndvi_vec[i]-x_test[1])/
                                    (x_test[2]-x_test[1])) + d(rng);
    std::cout << ndvi_vec[i] << "\t" << lai_vec[i] << std::endl;
    }

  lai_log_ndvi laiFs;
  pheno::ParameterCostFunction<lai_log_ndvi> fs{params, nb_measures, lai_vec, ndvi_vec, laiFs};

  VectorType x(params);
  x[0] = 2.0;
  x[1] = 1.0;
  x[2] = 0.0;

  auto err(pheno::optimize(x, fs));
  std::cout << err << std::endl;
  std::cout << x << std::endl;

  return EXIT_SUCCESS;
}
