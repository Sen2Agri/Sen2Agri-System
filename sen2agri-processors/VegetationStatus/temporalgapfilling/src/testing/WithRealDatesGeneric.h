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
#ifndef WRDG_H
#define WRDG_H
#include "otbGapfillingTests.h"

template<typename GFF>
int withRealDates(bool check_values)
{
  TestDataType trtrd = {
    // Each element is a tuple (mask, pixel, dates, outputdates, expected)
    std::make_tuple(VectorType{0, 0, 0, 0, 0},
                    VectorType{1, 1, 1, 1, 1},
                    VectorType{20130131, 20130205, 20130210, 20130225, 
                        20130307}, //input dates
                    VectorType{20130131, 20130206, 20130212, 20130218, 20130224, 
                        20130301}, //output dates
                    VectorType{1, 1, 1, 1, 1, 1}),
    std::make_tuple(VectorType{0, 0, 0, 0, 0},
                    VectorType{1, 1, 1, 1, 1},
                    VectorType{20130131, 20130205, 20130210, 20130225, 
                        20130307}, //input dates
                    VectorType{20130131, 20130206, 20130212, 20130218, 20130224, 
                        20130301, 20130308}, //output dates
                    VectorType{1, 1, 1, 1, 1, 1, 1}),
    std::make_tuple(VectorType{0, 0, 0, 0, 0},
                    VectorType{1, 1, 1, 1, 1},
                    VectorType{20130131, 20130205, 20130210, 20130225, 
                        20130307}, //input dates
                    VectorType{20130125, 20130131, 20130206, 20130212, 20130218,
                        20130224, 20130301, 20130308}, //output dates
                    VectorType{1, 1, 1, 1, 1, 1, 1, 1})
  };

  for(auto t : trtrd)
    {
    auto m = std::get<0>(t);
    auto p = std::get<1>(t);
    auto d = std::get<2>(t);
    auto o = std::get<3>(t);
    auto e = std::get<4>(t);

    //convert the dates to doys
    auto conversion_function = [](ValueType dd)
      {
        return pheno::doy(pheno::make_date(std::to_string(dd)));
      };
    std::vector<ValueType> idoy_vector(d.size(), ValueType{0});
    std::transform(std::begin(d), std::end(d),
                   std::begin(idoy_vector), 
                   conversion_function);
    std::vector<ValueType> odoy_vector(o.size(), ValueType{0});
    std::transform(std::begin(o), std::end(o),
                   std::begin(odoy_vector), 
                   conversion_function);
    
    PixelType mask(m.data(), m.size());
    PixelType pix(p.data(), p.size());
    PixelType dat(idoy_vector.data(), idoy_vector.size());
    PixelType odat(odoy_vector.data(), odoy_vector.size());
    PixelType expect(e.data(), e.size());

    std::cout << "input doys " << dat << std::endl
              << "output doys " << odat << std::endl;

    GFF gfd{dat, odat};
    
    auto res = gfd(pix, mask);
    if(check_values && res!=expect)
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
#endif
