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
#include <fstream>
#include <tuple>
#include "otbImageFileWriter.h"

using ImageType = otb::VectorImage<ValueType, 2>;
using LinearFunctorType =
  GapFilling::LinearGapFillingFunctor<ImageType::PixelType>;

void create_image(std::string ima_name, PixelType pix)
{
  ImageType::IndexType start = {0,0};
  ImageType::SizeType size = {1,1};
  ImageType::RegionType region;
  region.SetSize(size);
  region.SetIndex(start);

  auto in_image = ImageType::New();
  in_image->SetRegions(region);
  in_image->SetNumberOfComponentsPerPixel(pix.GetSize());
  in_image->Allocate();
  in_image->SetPixel(start, pix);

  auto writer = otb::ImageFileWriter<ImageType>::New();
  writer->SetInput(in_image);
  writer->SetFileName(ima_name);
  writer->Update();
}
std::tuple<std::string, std::string, std::string, std::string, std::string> 
create_data_files(std::string prefix, size_t cpd)
{
 
  unsigned int nbDates = 0;
  unsigned int nbOutputDates = 0;
  auto date_file = prefix+"_dates.txt";
  std::ofstream df(date_file);
  df << "20140101\n"; nbDates++;
  df << "20140106\n"; nbDates++;
  df << "20140112\n"; nbDates++;
  df << "20140115\n"; nbDates++;
  df.close();
  auto out_date_file = prefix+"_out_dates.txt";
  std::ofstream odf(out_date_file);
  odf << "20140101\n"; nbOutputDates++;
  odf << "20140106\n"; nbOutputDates++;
  odf << "20140109\n"; nbOutputDates++;
  odf << "20140112\n"; nbOutputDates++;
  odf << "20140115\n"; nbOutputDates++;
  odf.close();

  unsigned int nbComponents(nbDates*cpd);
  PixelType pix{nbComponents};
  for(size_t i=0; i<nbComponents; i++)
    pix[i] = i;

  for(size_t i=0; i<cpd; i++)
    pix[cpd*2+i] = 20;
  auto in_image_file = prefix+"_in_image.tif";
  create_image(in_image_file,pix);

  auto mask_file = prefix+"_mask.tif";
  pix.Fill(0);
  for(size_t i=0; i<cpd; i++)
    pix[cpd*2+i] = 1;
  create_image(mask_file,pix);

  auto out_image_file = prefix+"_out_image.tif";
  return std::make_tuple(in_image_file, mask_file, out_image_file, date_file, 
                         out_date_file);
}

int imageFunctionGapfillingTest(int argc, char * argv[])
{
  if(argc!=3)
    {
    std::cout << argc-1 << " arguments given" << std::endl;
    std::cout << "Usage: " << argv[0] << " file_path components_per_date" 
              << std::endl;
    return EXIT_FAILURE;
    }

  size_t cpd = std::atoi(argv[2]);
  std::string in_image_file, mask_file, out_image_file, date_file, out_date_file;

  std::stringstream prefix;
  prefix << std::string(argv[1])+"/id" << cpd;
  std::tie(in_image_file, mask_file, out_image_file, date_file, out_date_file) = 
    create_data_files(prefix.str(), cpd);
  std::cout << "Only input dates" << std::endl;
  GapFilling::gapfill_time_series<ImageType, LinearFunctorType>(in_image_file,
                                                                mask_file,
                                                                out_image_file,
                                                                cpd,
                                                                date_file);
  std::stringstream prefix2;
  prefix2 << std::string(argv[1])+"/od" << cpd;
  std::tie(in_image_file, mask_file, out_image_file, date_file, out_date_file) = 
    create_data_files(prefix2.str(),cpd);
  std::cout << "With output dates" << std::endl;
  GapFilling::gapfill_time_series<ImageType, LinearFunctorType>(in_image_file,
                                                                mask_file,
                                                                out_image_file,
                                                                cpd,
                                                                date_file,
                                                                out_date_file);
  return EXIT_SUCCESS;
}
