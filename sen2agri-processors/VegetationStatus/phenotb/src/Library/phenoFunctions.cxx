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

#include "phenoFunctions.h"
#include "dateUtils.h"
#include <algorithm>

namespace pheno
{


// normalized_sigmoid
namespace normalized_sigmoid{


}// normalized sigmoid

namespace STICS{
VectorType F(const VectorType& t, const VectorType& x)
{
  VectorType y(t.size());
  auto K = x[0];
  auto Ti = x[1];
  auto Tf = x[2];
  auto a = x[4];
  auto b = x[5];
  auto St = 0.0;
  for(size_t i=0; i<t.size(); ++i)
    {
    St += t[i];
    y[i] = K*((1.0/(1.0+exp(-b*(St-Ti))))-exp(a*(St-Tf)));
    }
  return y;
}
} //STICS

 // sigmoid

namespace gaussian{
VectorType F(const VectorType& t, const VectorType& x)
{
  VectorType y(t.size());
  for(size_t i=0; i<t.size(); ++i)
    y[i] = x[2]*(exp(-vnl_math_sqr((t[i]-x[0])/x[1])))+x[3];
  return y;
}
VectorType guesstimator(VectorType p, VectorType t)
{
  // Find the max and min elements
  auto minmax = std::minmax_element(std::begin(p), std::end(p));
  auto vmax = *(minmax.second);
  auto vmin = *(minmax.first);
  auto pmax = minmax.second - std::begin(p);
  VectorType x0(4);
  x0[0] = t[pmax];
  x0[1] = 30.0;
  x0[2] = (vmax-vmin);
  x0[3] = vmin; 
  return x0;
}

}// gaussian

double gaussianFunction(double x, double m, double s)
{
  return exp(-vnl_math_sqr((x-m)/(s)/2.0));
}


}// pheno
