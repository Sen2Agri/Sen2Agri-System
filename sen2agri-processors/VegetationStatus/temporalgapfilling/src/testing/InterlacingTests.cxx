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

std::vector<std::tuple<VectorType, VectorType, VectorType, VectorType> > 
ditd{ 
  std::make_tuple(VectorType{1, 2, 3, 4, 5},
                  VectorType{1, 2, 3, 4, 5},
                  VectorType{1, 3, 5},
                  VectorType{1, 3, 5}),
    std::make_tuple(VectorType{1, 2, 23, 4, 5},
                    VectorType{1, 2, 3, 4, 5},
                    VectorType{1, 2, 3, 4, 5},
                    VectorType{1, 2, 23, 4, 5}),
    std::make_tuple(VectorType{1, 2, 23, 4, 5},
                    VectorType{1, 2, 3, 4, 5},
                    VectorType{1, 2, 3, 5},
                    VectorType{1, 2, 23, 5}),
    std::make_tuple(VectorType{0, 1, 2, 0, 4, 5, 6},  //pixel
                    VectorType{0, 1, 2, 3, 4, 5, 6}, //input dates
                    VectorType{0, 1, 2, 4, 5, 6}, //output dates
                    VectorType{0, 1, 2, 4, 5, 6}),
  std::make_tuple(VectorType{0, 0, 1, 0, 2, 0, 0, 4, 0, 5, 0, 6, 0}, //pix
                  VectorType{0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6}, //id
                  VectorType{0, 1, 2, 3, 4, 5, 6}, //output dates
                  VectorType{0, 1, 2, 0, 4, 5, 6}),
    std::make_tuple(VectorType{1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0},//pix
                    VectorType{0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6},//id
                    VectorType{0, 1, 2, 3, 4, 5, 6},//od
                    VectorType{1, 1, 1, 0, 1, 1, 1})
    };

int deinterlacingTest(int argc, char * argv[])
{
 if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  for(auto& t : ditd)
    {
    auto p = std::get<0>(t);  
    auto d = std::get<1>(t);  
    auto od = std::get<2>(t);  
    auto e = std::get<3>(t);  

    PixelType pix(p.data(), p.size());
    PixelType dv(d.data(), d.size());
    PixelType odv(od.data(), od.size());
    PixelType expected(e.data(), e.size());
    auto res = GapFilling::extract_output_dates(pix, dv, odv);
    if(expected !=  res)
      {
      std::cout << "p" << pix << std::endl;
      std::cout << "d" << dv << std::endl;
      std::cout << "o" << odv << std::endl;
      std::cout << "e" << expected << std::endl;;
      std::cout << "r" << res << std::endl;
      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}

std::vector<std::tuple<VectorType, VectorType, VectorType, 
                       VectorType, VectorType, VectorType, VectorType> > 
itd{ 
  std::make_tuple(VectorType{1, 2, 23, 4, 5}, // pix
                  VectorType{0, 0, 0, 0, 0}, // imask
                  VectorType{1, 20, 30, 40, 50},  // idv
                  VectorType{10, 25, 35},     // odv
                  VectorType{1, 0,   2,  0, 23,  0,  4,  5},   //expect pix
                  VectorType{0, 1,   0,  1,  0,  1,  0,  0},   //expect mask
                  VectorType{1, 10, 20, 25, 30, 35, 40, 50}),    //expect dv
    std::make_tuple(VectorType{1,  2, 23,  4,  5}, // pix
                    VectorType{0,  0,  0,  0,  0}, // imask
                    VectorType{1, 20, 30, 40, 50},  // idv
                    VectorType{10, 25, 27, 35, 37, 45, 47},     // odv
                    VectorType{1, 0,   2,  0,  0, 23,  0,  0,  4,  0,  0,  5},   //expect pix
                    VectorType{0, 1,   0,  1,  1,  0,  1,  1,  0,  1,  1,  0},   //expect mask
                    VectorType{1, 10, 20, 25, 27, 30, 35, 37, 40, 45, 47, 50}),    //expect dv
    std::make_tuple(VectorType{1,  2, 23,  4,  5}, // pix
                    VectorType{0,  0,  0,  0,  0}, // imask
                    VectorType{10, 20, 30, 40, 50},  // idv
                    VectorType{1, 25, 27, 35, 37, 45, 47},     // odv
                    VectorType{0, 1,   2,  0,  0, 23,  0,  0,  4,  0,  0,  5},   //expect pix
                    VectorType{1, 0,   0,  1,  1,  0,  1,  1,  0,  1,  1,  0},   //expect mask
                    VectorType{1, 10, 20, 25, 27, 30, 35, 37, 40, 45, 47, 50}),    //expect dv
    std::make_tuple(VectorType{1, 2, 23, 4, 5}, // pix
                    VectorType{0, 0, 0, 0, 0}, // imask
                    VectorType{1, 20, 30, 40, 50},  // idv
                    VectorType{10, 25, 55},     // odv
                    VectorType{1, 0,   2,  0, 23, 4,  5, 0},   //expect pix
                    VectorType{0, 1,   0,  1,  0, 0,  0, 1},   //expect mask
                    VectorType{1, 10, 20, 25, 30, 40, 50, 55}),   //expect dv
  std::make_tuple(VectorType{0, 1, 2, 4, 5, 6},  //pixel
                  VectorType{0, 0, 0, 0, 0, 0},
                  VectorType{0, 1, 2, 4, 5, 6}, //input dates
                  VectorType{0, 1, 2, 3, 4, 5, 6}, //output dates
                  VectorType{0, 0, 1, 0, 2, 0, 0, 4, 0, 5, 0, 6, 0}, //exp pix
                  VectorType{0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1}, //exp mask
                  VectorType{0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6}),
    std::make_tuple(VectorType{1, 1, 1, 1, 1, 1},//pix
                    VectorType{0, 0, 0, 0, 0, 0},//imask
                    VectorType{0, 1, 2, 4, 5, 6},//idv
                    VectorType{0, 1, 2, 3, 4, 5, 6}, //odv
                    VectorType{1, 0, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0},//exp pix
                    VectorType{0, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1},//exp imask
                    VectorType{0, 0, 1, 1, 2, 2, 3, 4, 4, 5, 5, 6, 6}) //exp dv
    };

int interlacingTest(int argc, char * argv[])
{
 if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }


  typename PixelType::ValueType valid_value{0};
  for(auto& t : itd)
    {

    auto pv = std::get<0>(t);  
    auto mv = std::get<1>(t);  
    auto idv = std::get<2>(t);  
    auto odv = std::get<3>(t);  
    auto epv = std::get<4>(t);  
    auto emv = std::get<5>(t);  
    auto edv = std::get<6>(t);  

    PixelType p(pv.data(), pv.size());
    PixelType m(mv.data(), mv.size());
    PixelType id(idv.data(), idv.size());
    PixelType od(odv.data(), odv.size());
    PixelType ep(epv.data(), epv.size());
    PixelType em(emv.data(), emv.size());
    PixelType ed(edv.data(), edv.size());

    PixelType tmp_pix, tmp_mask, tmp_dates;
    std::tie(tmp_pix, tmp_mask, tmp_dates) = 
      GapFilling::create_tmp_data_interlace_dates(p, m, id, od, valid_value);
    if(tmp_pix != ep || tmp_mask != em || tmp_dates != ed)
      {

      std::cout << "-- pixel --" << std::endl;
      std::cout << tmp_pix << std::endl;
      std::cout << ep << std::endl;

      std::cout << "-- mask --" << std::endl;
      std::cout << tmp_mask << std::endl;
      std::cout << em << std::endl;

      std::cout << "-- dates --" << std::endl;
      std::cout << tmp_dates << std::endl;
      std::cout << ed << std::endl;

      return EXIT_FAILURE;
      }
    }
  return EXIT_SUCCESS;
}
