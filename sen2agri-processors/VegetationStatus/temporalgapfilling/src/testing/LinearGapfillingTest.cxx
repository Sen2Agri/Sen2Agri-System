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
#include "WithRealDatesGeneric.h"

using GFF = typename GapFilling::LinearGapFillingFunctor<PixelType>;

// Data for tests
TestDataType tdt = {
  // Each element is a tuple (mask, pixel, dates, outputdates, expected)
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0, 0},
                  VectorType{1, 1, 2, 3, 4, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{1, 1, 2, 3, 4, 1, 1}),
  std::make_tuple(VectorType{1, 1, 1, 1, 1, 1, 1},
                  VectorType{1, 1, 2, 3, 4, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 0, 0, 0, 0, 0, 0}),
  std::make_tuple(VectorType{0, 0, 0, 1, 0, 0, 0},
                  VectorType{1, 1, 1, 1, 1, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{1, 1, 1, 1, 1, 1, 1}),
  std::make_tuple(VectorType{1, 0, 0, 0, 0, 0, 0},
                  VectorType{1, 1, 1, 1, 1, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{1, 1, 1, 1, 1, 1, 1}),
  std::make_tuple(VectorType{1, 1, 1, 0, 0, 0, 0},
                  VectorType{0, 0, 1, 1, 1, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{1, 1, 1, 1, 1, 1, 1}),
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 1, 1},
                  VectorType{1, 1, 1, 1, 1, 20, 0},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{1, 1, 1, 1, 1, 1, 1}),
  std::make_tuple(VectorType{0, 0, 0, 1, 0, 0, 0},
                  VectorType{1, 1, 2, 3, 4, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{1, 1, 2, 3, 4, 1, 1})
};

TestDataType tdt2 = {
  // Each element is a tuple (mask, pixel, dates, outputdates, expected)
  std::make_tuple(VectorType{0, 0, 0, 1, 0, 0, 0},
                  VectorType{1, 1, 1, 0, 4, 1, 1},
                  VectorType{0, 1, 2, 2.5, 4, 5, 6},
                  VectorType{0, 1, 2, 2.5, 4, 5, 6},
                  VectorType{1, 1, 1, 7.0/4, 4, 1, 1}),
  std::make_tuple(VectorType{0, 0, 0, 1, 0, 0, 0},
                  VectorType{1, 1, 1, 0, 4, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{1, 1, 1, 3.0/2*3-2, 4, 1, 1}),
  std::make_tuple(VectorType{0, 0, 0, 1, 0, 0, 0},
                  VectorType{1, 1, 1, 0, 4, 1, 1},
                  VectorType{0, 1, 2, 3.7, 4, 5, 6},
                  VectorType{0, 1, 2, 3.7, 4, 5, 6},
                  VectorType{1, 1, 1, 3.0/2*3.7-2, 4, 1, 1}),
  std::make_tuple(VectorType{0, 0, 0, 1, 0, 0, 0},
                  VectorType{1, 1, 1, 0, 4, 1, 1},
                  VectorType{0, 1, 2.5, 3.7, 4, 5, 6},
                  VectorType{0, 1, 2.5, 3.7, 4, 5, 6},
                  VectorType{1, 1, 1, 2*3.7-4, 4, 1, 1}),
  std::make_tuple(VectorType{0, 1, 0, 1, 0, 0, 0},
                  VectorType{1, 0, 1, 0, 4, 1, 1},
                  VectorType{1, 1, 2.5, 3.7, 4, 5, 6},
                  VectorType{1, 1, 2.5, 3.7, 4, 5, 6},
                  VectorType{1, 1, 1, 2*3.7-4, 4, 1, 1}),
  std::make_tuple(VectorType{0, 0, 0, 1, 0, 0, 0},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6})
};



int linearGapfillingTest(int argc, char * argv[])
{
 if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

// regular dates
  for(auto t : tdt)
    {
    auto m = std::get<0>(t);
    auto p = std::get<1>(t);
    auto d = std::get<2>(t);
    auto e = std::get<4>(t);

    GFF gf{};

    PixelType mask(m.data(), m.size());
    PixelType pix(p.data(), p.size());
    PixelType dat(d.data(), d.size());
    PixelType expect(e.data(), e.size());
    auto res = gf(pix, mask);

    if(res!=expect)
      {
      std::cout << "-- without dates --" << std::endl;
      std::cout << mask << std::endl;
      std::cout << pix << std::endl;
      std::cout << res << std::endl;
      return EXIT_FAILURE;
      }

    GFF gfd{dat};
    res = gfd(pix, mask);
    if(res!=expect)
      {
      std::cout << "-- with dates --" << std::endl;
      std::cout << mask << std::endl;
      std::cout << pix << std::endl;
      std::cout << dat << std::endl;
      std::cout << res << std::endl;
      return EXIT_FAILURE;
      }
    }
//irregular dates
  for(auto t : tdt2)
    {
    auto m = std::get<0>(t);
    auto p = std::get<1>(t);
    auto d = std::get<2>(t);
    auto e = std::get<4>(t);
    
    PixelType mask(m.data(), m.size());
    PixelType pix(p.data(), p.size());
    PixelType dat(d.data(), d.size());
    PixelType expect(e.data(), e.size());
    GFF gfd{dat};
    auto res = gfd(pix, mask);
    if(res!=expect)
      {
      std::cout << "-- with irregular dates --" << std::endl;
      std::cout << mask << std::endl;
      std::cout << pix << std::endl;
      std::cout << dat << std::endl;
      std::cout << res << std::endl;
      std::cout << expect << std::endl;
      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}

TestDataType tdt3 = {
  // Each element is a tuple (mask, pixel, dates, outputdates, expected)
  std::make_tuple(VectorType{0, 0, 0, 1, 0, 0, 0},
                  VectorType{1, 1, 1, 0, 4, 1, 1},
                  VectorType{0, 1, 2, 2.5, 4, 5, 6}, //input dates
                  VectorType{0, 1, 2, 2.5, 4, 5, 6}, //output dates
                  VectorType{1, 1, 1, 7.0/4, 4, 1, 1}),
  std::make_tuple(VectorType{0, 0, 0, 1, 0, 0, 0},
                  VectorType{1, 1, 1, 0, 4, 1, 1},
                  VectorType{0, 1, 2, 2.5, 4, 5, 6}, //input dates
                  VectorType{0, 1, 2, 4, 5, 6}, //output dates
                  VectorType{1, 1, 1, 4, 1, 1}),
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0, 0},
                  VectorType{0, 1, 2, 3, 4, 5, 6},  //pixel
                  VectorType{0, 1, 2, 3, 4, 5, 6}, //input dates
                  VectorType{0, 1, 2, 4, 5, 6}, //output dates
                  VectorType{0, 1, 2, 4, 5, 6}),
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0},
                  VectorType{0, 1, 2, 4, 5, 6},  //pixel
                  VectorType{0, 1, 2, 4, 5, 6}, //input dates
                  VectorType{0, 1, 2, 3, 4, 5, 6}, //output dates
                  VectorType{0, 1, 2, 3, 4, 5, 6})
};

int linearWithOutputDatesGapfillingTest(int argc, char * argv[])
{
 if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

// regular dates
  for(auto t : tdt3)
    {
    auto m = std::get<0>(t);
    auto p = std::get<1>(t);
    auto d = std::get<2>(t);
    auto o = std::get<3>(t);
    auto e = std::get<4>(t);
    
    PixelType mask(m.data(), m.size());
    PixelType pix(p.data(), p.size());
    PixelType dat(d.data(), d.size());
    PixelType odat(o.data(), o.size());
    PixelType expect(e.data(), e.size());
    GFF gfd{dat, odat};
    auto res = gfd(pix, mask);

    if(res!=expect)
      {
      std::cout << "m" << mask << std::endl;
      std::cout << "p" << pix << std::endl;
      std::cout << "d" << dat << std::endl;
      std::cout << "o" << odat << std::endl;
      std::cout << "e" << expect << std::endl;;
      std::cout << "r" << res << std::endl;
      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}

int linearWithRealDates(int argc, char * argv[])
{
 if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  return withRealDates<GFF>(true);
}

