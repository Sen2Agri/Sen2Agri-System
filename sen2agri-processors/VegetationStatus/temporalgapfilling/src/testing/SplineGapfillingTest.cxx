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

using GFF = typename GapFilling::SplineGapFillingFunctor<PixelType>;
const double tolerance = 10e-3;
double dist(PixelType p1, PixelType p2)
{
  double res{0};
  auto pixelSize = p1.GetSize();
  for(size_t i=0; i<pixelSize; i++)
    res += fabs(p1[i]-p2[i]);
  return res;
}
// Data for tests
TestDataType tdt_splines = {
  // Each element is a tuple (mask, pixel, dates, outputdates, expected)
  std::make_tuple(VectorType{0, 0, 0, 0, 0, 0, 0},
                  VectorType{1, 1, 2, 3, 4, 1, 1},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{0, 1, 2, 3, 4, 5, 6},
                  VectorType{1, 1, 2, 3, 4, 1, 1}),
  std::make_tuple(VectorType{0,0,0,0,0,0,0,0,0,0},
                  VectorType{1,1.54030,1.34636,2.08887,3.04234,5.99120,5.87204,7.30059,8.39186,9.77669},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{1,1.54030,1.34636,2.08887,3.04234,5.99120,5.87204,7.30059,8.39186,9.77669}),
  std::make_tuple(VectorType{0,       0,     0,      0,      1,      0,      0,      0,      0,     0},
                  VectorType{1,1.54030,1.34636,2.08887,    0.0,5.99120,5.87204,7.30059,8.39186,9.77669},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{1,1.54030,1.34636,2.08887,3.57275,5.9912,5.872040,7.30059,8.39186,9.77669}),
  std::make_tuple(VectorType{0,      0,      1,      0,      1,      0,      1,      0,      0,      0},
                  VectorType{1,1.54030,    0.0,2.08887,    0.0,5.99120,    0.0,7.30059,8.39186,9.77669},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{1,1.54030,1.87883,2.08887,3.32372,5.99120,6.64333,7.30059,8.39186,9.77669}),
  std::make_tuple(VectorType{1,      0,      1,      0,      1,      0,      1,      0,      0,      0}, //invalid 1st value
                  VectorType{1,1.54030,    0.0,2.08887,    0.0,5.99120,    0.0,7.30059,8.39186,9.77669},
                  VectorType{0,     1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{0,     1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{1.5403,1.54030,1.42053,2.08887,3.57030,5.99120,6.64333,7.30059,8.39186,9.77669}),
  std::make_tuple(VectorType{0,      1,      1,      1,      1,      1,      1,      1,      1,      0}, //linear
                  VectorType{1,1.54030,    0.0,2.08887,    0.0,5.99120,    0.0,7.30059,8.39186,9.77669},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{1,2.35448,3.34017,3.92735,4.45269,5.30970,6.58697,7.98669,9.09849,9.77669}),
  std::make_tuple(VectorType{0,      1,      1,      1,      1,      1,      1,      1,      1,      1}, //no interpolation
                  VectorType{1,1.54030,    0.0,2.08887,    0.0,5.99120,    0.0,7.30059,8.39186,9.77669},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{0,1.42074,2.45465,3.07056,3.62160,4.52054,5.86029,7.32849,8.49468,9.20606},
                  VectorType{1,1.54030,    0.0,2.08887,    0.0,5.99120,    0.0,7.30059,8.39186,9.77669})};

int splineGapfillingTest(int argc, char * argv[])
{
 if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

    for(auto t : tdt_splines)
    {
    auto m = std::get<0>(t);
    auto p = std::get<1>(t);
    auto d = std::get<2>(t);
    auto e = std::get<4>(t);

    GFF gfd{pheno::vectorToPixel(d)};
    PixelType mask(m.data(), m.size());
    PixelType pix(p.data(), p.size());
    PixelType dat(d.data(), d.size());
    PixelType expect(e.data(), e.size());

    auto res = gfd(pix, mask);
    if(dist(res,expect)>tolerance)
      {
      std::cout << "-- with dates --" << std::endl;
      std::cout << mask << std::endl;
      std::cout << dat << std::endl;
      std::cout << pix << std::endl;
      std::cout << expect << std::endl;
      std::cout << res << std::endl;
      return EXIT_FAILURE;
      }
    }
    return EXIT_SUCCESS;
}

int splineWithRealDates(int argc, char * argv[])
{
 if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  return withRealDates<GFF>(false);
}
