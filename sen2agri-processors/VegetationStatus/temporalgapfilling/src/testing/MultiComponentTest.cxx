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

  Program:   gapfilling
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See gapfilling-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "otbGapfillingTests.h"

using IGFF = typename GapFilling::IdentityGapFillingFunctor<PixelType>;
using LGFF = typename GapFilling::LinearGapFillingFunctor<PixelType>;
using SGFF = typename GapFilling::SplineGapFillingFunctor<PixelType>;

MCTestDataType tdtmc = {
  // Each element is a tuple (mask, pixel, dates)
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0, 0},
                  VectorType{1, 2, 1, 3, 2, 4, 3, 5, 4, 6, 1, 2, 1, 2},
                  VectorType{0, 1, 2, 3, 4, 5, 6}),
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0, 0},
                  VectorType{1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 1, 1, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6}),
  // test a mask with several components per date
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
                  VectorType{1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 1, 1, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6})
};

template <typename FunctorType>
int multiComponentGapfillingTestGeneric()
{
  using MCGFF = typename 
    GapFilling::MultiComponentTimeSeriesFunctorAdaptor<PixelType, FunctorType>;
  unsigned int test_case{0};
  for(auto& t : tdtmc)
    {
    test_case++;
    std::cout << "Processing test case nb " << test_case << std::endl;
    auto m = std::get<0>(t);
    auto p = std::get<1>(t);
    auto d = std::get<2>(t);

    //Multi-component gapfilling
    size_t cpp = p.size()/d.size();
    MCGFF mcgf{};
    mcgf.SetNumberOfComponentsPerDate(cpp);
    PixelType mask(m.data(), m.size());
    PixelType pix(p.data(), p.size());
    auto res_multi = mcgf(pix, mask);
  
    //Mono-component gapfilling
    std::cout << "cpp = " << cpp << std::endl;
    //Split the components of the mc pixel
    PixelType res_mono(p.data(),p.size());
    for(size_t i=0; i<cpp; i++)
      {
      PixelType pc(d.size());
      PixelType mc(d.size());
      for(size_t j=0; j<d.size(); j++)
        {
        pc[j]=pix[i+cpp*j];
        if(m.size()==p.size())
          mc[j]=mask[i+cpp*j];
        else
          mc[j]=mask[j];
        std::cout << mc[j] << std::endl;
        }
      FunctorType gf{};
      auto res = gf(pc, mc);
      for(size_t j=0; j<d.size(); j++)
        res_mono[i+cpp*j]=res[j];
      }
    if(res_mono != res_multi)
      {
      const double epsilon = 10e-3;
      double err{0.0};
      for(size_t i=0; i<cpp; i++)
        {
        std::cout << " -------- " << i <<  " -------- " << std::endl;
        for(size_t j=0; j<m.size(); j++)
          {
          std::cout << pix[i+cpp*j] << " -> "
                    << res_mono[i+cpp*j] << " vs " << res_multi[i+cpp*j]
                    << std::endl;
          err += fabs(res_mono[i+cpp*j]-res_multi[i+cpp*j]);
          }
        }
      if(err>epsilon)
        {
        std::cout << "Error = " << err << std::endl;
        return EXIT_FAILURE;
        }
      }
    }
  return EXIT_SUCCESS;
}

int multiComponentGapfillingTest(int argc, char * argv[])
{
  if(argc>2)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  switch(std::atoi(argv[1])){
  case 0:
    std::cout << "Identity functor" << std::endl;
    multiComponentGapfillingTestGeneric<IGFF>();
    break;
  case 1:
    std::cout << "Linear functor" << std::endl;
    multiComponentGapfillingTestGeneric<LGFF>();
    break;
  case 2:
    std::cout << "Spline functor" << std::endl;
    multiComponentGapfillingTestGeneric<SGFF>();
    break;
  default:
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
