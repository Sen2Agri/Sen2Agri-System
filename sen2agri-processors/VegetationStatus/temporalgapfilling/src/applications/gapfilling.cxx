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

  Copyright (c) CESBIO. All rights reserved.

  See gapfilling-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <cstdlib>
#include "gapfilling.h"

using ImageType = otb::VectorImage<float, 2>;
using LinearFunctorType =
  GapFilling::LinearGapFillingFunctor<ImageType::PixelType>;
using SplineFunctorType =
  GapFilling::SplineGapFillingFunctor<ImageType::PixelType>;

int main(int argc, char** argv)
{
  if(argc<6 || argc>8)
    {
    std::cerr << "Usage: " << argv[0] << " ndvi_series mask_series output_series "
              << "components_per_date interpolation_type (0: linear, 1: spline)"
              << " [input_date_file output_date_file]"
              << std::endl;
    std::cerr << "Example: " << argv[0]
              << " /home/inglada/stok/SudOuest2008/TestsInterpolation/ndvi.hdr "
              << "/home/inglada/stok/SudOuest2008/TestsInterpolation/masks.hdr "
              << "/home/inglada/stok/SudOuest2008/TestsInterpolation/test.tif 1 1"
              << std::endl;
    return EXIT_FAILURE;
    }
  auto cpd = std::atoi(argv[4]);
  std::string in_date_file{""};
  std::string out_date_file{""};
  if(argc>6)
    in_date_file = argv[6];
  if(argc>7)
    out_date_file = argv[7];
  if(!std::atoi(argv[5]))
    {
    std::cout << "Using linear interpolation and " << cpd
              << " components per date " << std::endl;
    GapFilling::gapfill_time_series<ImageType, LinearFunctorType>(argv[1],
                                                                  argv[2],
                                                                  argv[3],
                                                                  cpd,
                                                                  in_date_file,
                                                                  out_date_file);
    }
  else
    {
    std::cout << "Using spline interpolation and " << cpd
              << " components per date " << std::endl;
    GapFilling::gapfill_time_series<ImageType, SplineFunctorType>(argv[1],
                                                                  argv[2],
                                                                  argv[3],
                                                                  cpd,
                                                                  in_date_file,
                                                                  out_date_file);
    }
  return EXIT_SUCCESS;
}
