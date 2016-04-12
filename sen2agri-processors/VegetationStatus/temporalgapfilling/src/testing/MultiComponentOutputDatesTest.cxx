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

MCTestOutDatesDataType tdtmcod = {
  // Each element is a tuple (mask, pixel, input dates, output dates)
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0, 0},
                  VectorType{1, 2, 1, 3, 2, 4, 3, 5, 4, 6, 1, 2, 1, 2},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6}),
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0, 0},
                  VectorType{1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 1, 1, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6}),
  // test a mask with several components per date
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
                  VectorType{1, 1, 1, 1, 2, 2, 3, 3, 4, 4, 1, 1, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6}),
  // different number of output dates
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0},
                  VectorType{1, 1, 1, 1, 3, 3, 4, 4, 1, 1, 1, 1},
                  VectorType{0, 1, 2, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6})
};

template<typename InterpolType>
int multiComponentOutputDatesGapfillingTestGeneric()
{
  using MCGFF = typename 
    GapFilling::MultiComponentTimeSeriesFunctorAdaptor<PixelType, InterpolType>;
  unsigned int test_case = 0;
  for(auto& t : tdtmcod)
    {
    test_case++;
    std::cout << " --------------- " << test_case << " --------------- " 
              << std::endl;
    auto m = std::get<0>(t);
    auto p = std::get<1>(t);
    auto d = std::get<2>(t);
    auto o = std::get<3>(t);

    //Multi-component gapfilling
    size_t cpp = p.size()/d.size();
    PixelType in_dates(d.data(), d.size());
    PixelType out_dates(o.data(), o.size());
    MCGFF mcgf{};
    mcgf.SetFunctor(InterpolType(in_dates, out_dates));
    mcgf.SetNumberOfComponentsPerDate(cpp);
    PixelType mask(m.data(), m.size());
    PixelType pix(p.data(), p.size());
    auto res_multi = mcgf(pix, mask);
    std::cout << "---> out functor " << std::endl;
    if(res_multi.GetSize() != out_dates.GetSize()*cpp)
      {
      std::cout << "Result has the wrong size: " 
                << res_multi.GetSize() << " instead of " 
                << out_dates.GetSize()*cpp << std::endl;
      return EXIT_FAILURE;
      }
  
    //Mono-component gapfilling
    std::cout << "cpp = " << cpp << std::endl;
    //Split the components of the mc pixel
    PixelType res_mono(res_multi.GetSize());
    for(size_t i=0; i<cpp; i++)
      {
      PixelType pc(d.size());
      PixelType mc(d.size());
      for(size_t j=0; j<d.size(); j++)
        pc[j]=pix[i+cpp*j];
      if(mask.GetSize()==pix.GetSize())
        for(size_t j=0; j<d.size(); j++)
          mc[j]=mask[i+cpp*j];
      else mc=mask;
      std::cout << "pc " << pc << std::endl;
      std::cout << "mc " << mc << std::endl;
      InterpolType gf{in_dates, out_dates};
      auto res = gf(pc, mc);
      std::cout << "---> out functor " << std::endl;
      for(size_t j=0; j<out_dates.GetSize(); j++)
        res_mono[i+cpp*j]=res[j];
      }
    if(res_mono != res_multi)
      {
      for(size_t i=0; i<cpp; i++)
        {
        std::cout << " -------- " << i <<  " -------- " << std::endl;
        for(size_t j=0; j<out_dates.GetSize(); j++)
          std::cout << res_mono[i+cpp*j] << " vs " << res_multi[i+cpp*j]
                    << std::endl;
        }
      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}

int multiComponentOutputDatesGapfillingTest(int argc, char * argv[])
{
  if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  if( multiComponentOutputDatesGapfillingTestGeneric<LGFF>() ==
      EXIT_FAILURE)
    {
    std::cout << "Error in linear interpolation" << std::endl;
    return EXIT_FAILURE;
    }

  if( multiComponentOutputDatesGapfillingTestGeneric<SGFF>() ==
      EXIT_FAILURE)
    {
    std::cout << "Error in spline interpolation" << std::endl;
    return EXIT_FAILURE;
    }

  return EXIT_SUCCESS;
}
