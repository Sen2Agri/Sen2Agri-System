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
#include <tuple>

using PrecisionType = double;
using VectorType = pheno::VectorType;
using PixelType = itk::VariableLengthVector<PrecisionType>;
using FunctorType = pheno::TwoCycleSigmoFittingFunctor<PixelType>;

template <typename T>
inline
std::pair<bool,double>  err(T x, T y)
{
  double e = fabs((x-y)/(y+10e-10));
  if (e > 1) return std::make_pair(true,e);
  return std::make_pair(false,e);
}

int phenoTwoCycleFittingFunctor(int argc, char * argv[])
{
  if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  size_t nb_dates{20};
  std::vector<std::tm> dv{nb_dates};
  auto ddv = VectorType{static_cast<unsigned int>(nb_dates)};
  auto mm1 = std::make_pair(1, 100);
  VectorType x_1(4);
  x_1[0] = 50.0;
  x_1[1] = 7.0;
  x_1[2] = 200.0;
  x_1[3] = 5.0;
  auto mm2 = std::make_pair(1, 10);
  VectorType x_2(4);
  x_2[0] = 250.0;
  x_2[1] = 7.0;
  x_2[2] = 320.0;
  x_2[3] = 5.0;
  for(size_t i=0; i<nb_dates; i++)
    {
    std::tm dd;
    dd.tm_hour = dd.tm_min = dd.tm_sec = dd.tm_isdst = 0;
    dd.tm_year = 2012;
    dd.tm_mon = static_cast<unsigned int>((12*i)/nb_dates);
    dd.tm_mday = 1 + static_cast<unsigned int>((28*i)/nb_dates) ;
    dv[i] = dd;
    }

  const auto &days = pheno::tm_to_doy_list(dv);
  std::copy(std::begin(days), std::end(days), std::begin(ddv));

  VectorType tmps1(nb_dates);
  VectorType tmps2(nb_dates);
  pheno::normalized_sigmoid::F<VectorType>()(ddv, x_1, tmps1);
  pheno::normalized_sigmoid::F<VectorType>()(ddv, x_2, tmps2);
  auto tmpres = tmps1*(mm1.second-mm1.first)+mm1.first
    + tmps2*(mm2.second-mm2.first)+mm2.first;
  PixelType pixel(nb_dates);
  PixelType mask(nb_dates);
  for(size_t i=0; i<nb_dates; i++)
    {
    pixel[i] = tmpres[i];
    mask[i] = PrecisionType{0};
    }

  PixelType vecpars(12);
  vecpars[0] = (mm1.second-mm1.first);
  vecpars[1] = mm1.first;
  for(auto i=0; i<4; ++i)
    vecpars[i+2] = x_1[i];

  vecpars[6] = (mm2.second-mm2.first);
  vecpars[7] = mm2.first;
  for(auto i=0; i<4; ++i)
    vecpars[i+8] = x_2[i];


  FunctorType fun;
  fun.SetDates(dv);
  fun.SetReturnFit(false);
  fun.SetFitOnlyInvalid(false);

  auto result = fun(pixel,mask);

  for(auto p=0; p<6; ++p)
    {
    auto e = err(result[p], vecpars[p]);
    if(e.first)
      {
      std::cout << p << ": " << result[p] << "\t" << vecpars[p] << "\t" << e.second << std::endl;
      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}
