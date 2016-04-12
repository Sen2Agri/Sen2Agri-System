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

#include "vnl/vnl_random.h"
#include <iostream>
#include <sstream>
#include <limits>

#include "phenoFunctions.h"
#include "miscUtils.h"

int main()
{
  const unsigned int params{6};
  auto phF(pheno::sigmoid::F<pheno::VectorType>);
  const unsigned int nbDates{12};
  pheno::VectorType t(nbDates);

  vnl_random m_RNG{vnl_random()};

  double stdev{0.0};
  double stdev_max{0.5};
  double stdev_step{0.05};

  int nbSimus{20};

  double jitter{0.1};
  while(stdev<stdev_max)
    {

    double err_min{std::numeric_limits<double>::max()};
    double err_max{std::numeric_limits<double>::min()};
    double err_mean{0.0};
    for(auto simu=1; simu<=nbSimus; ++simu)
      {
      for(size_t i=0; i<nbDates; ++i)
        t[i] = i + m_RNG.normal64() * jitter;
  
      pheno::VectorType xt(params);
      xt[0] = nbDates/3.0 + m_RNG.normal64() * jitter;
      xt[1] = 2.0+ m_RNG.normal64() * jitter;
      xt[2] = 2*nbDates/3.0 + m_RNG.normal64() * jitter;
      xt[3] = 0.5+ m_RNG.normal64() * jitter;
      xt[4] = 3.0+ m_RNG.normal64() * jitter;
      xt[5] = 0.0+ m_RNG.normal64() * jitter;

      pheno::VectorType y(nbDates);
      phF(t,xt, y);
      pheno::VectorType yClean(y);
  
      for(size_t i=0; i<nbDates; ++i)
        {
        double noise{m_RNG.normal64() * stdev};
        if(i<xt[0] or i>xt[2])
          y[i] += noise;
        }
      // Set up a pheno::ParameterCostFunction compute object
      pheno::ParameterCostFunction<decltype(phF)> f{params, nbDates, y, t, phF};

      // Set up the initial guess
      double min{std::numeric_limits<double>::max()};
      double max{std::numeric_limits<double>::min()};
      double posmax{0};

      for(size_t i=0; i<nbDates; ++i)
        {
        if(y[i]>max)
          {
          max = y[i];
          posmax = t[i];
          }
        if(y[i]<min)
          {
          min = y[i];
          }
        }
  
      pheno::VectorType x0(params);
      x0[0] = posmax - (1+0.1*nbDates);
      x0[1] = 1.0;
      x0[2] = posmax + (1+0.1*nbDates);
      x0[3] = 1.0;
      x0[4] = (max-min);
      x0[5] = min;

      // Temp variable.
      pheno::VectorType x(x0);

      // The optimisation
      auto err(pheno::optimize(x, f));

      if(err < err_min) err_min = err;
      if(err > err_max) err_max = err;
      err_mean += (err/nbSimus);

      pheno::VectorType yHat(nbDates);
      phF(t,x,yHat);

      std::stringstream fileName("");
      fileName << "/tmp/testplot-" << stdev << "-" << simu <<  ".txt";
      pheno::generatePlotFile(fileName.str(), t, yClean, y, yHat, x, err);
      }
    std::cout << "Stdev = " << stdev << std::endl;
    std::cout << "Errors = " << err_min << "\t" << err_mean << "\t" << err_max << std::endl;
    std::cout <<"---------------------------------------------------\n";
    stdev += stdev_step;
    }
  
  return 0;
}


