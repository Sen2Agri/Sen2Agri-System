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

  Copyright (c) CESBIO. All rights reserved.

  See phenotb-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbStandardFilterWatcher.h"

#include "phenoFunctions.h"
#include "itkBinaryFunctorImageFilter.h"

int main(int argc, char** argv)
{

  if(argc!=5)
    {
    std::cerr << "Usage: " << argv[0] << " ndvi_series mask_series date_file output_series" << std::endl;
    std::cerr << "Example: " << argv[0] << " /home/inglada/stok/SudOuest2008/TestsInterpolation/ndvi.hdr /home/inglada/stok/SudOuest2008/TestsInterpolation/masks.hdr /home/inglada/stok/SudOuest2008/TestsInterpolation/dates.txt /home/inglada/stok/SudOuest2008/TestsInterpolation/test.tif" << std::endl;
    return EXIT_FAILURE;
    }
  
  using ImageType = otb::VectorImage<float, 2>;

  auto readerIma = otb::ImageFileReader<ImageType>::New();
  auto readerMask = otb::ImageFileReader<ImageType>::New();

  readerIma->SetFileName(argv[1]);
  readerMask->SetFileName(argv[2]);

  // prepare the vector of dates
  auto dates = pheno::parse_date_file(argv[3]);
  // pipeline

  using FunctorType = pheno::TwoCycleSigmoFittingFunctor<ImageType::PixelType>;

  auto filter = itk::BinaryFunctorImageFilter<ImageType, ImageType, ImageType, FunctorType>::New();
  readerIma->GetOutput()->UpdateOutputInformation();
  readerMask->GetOutput()->UpdateOutputInformation();
  filter->SetInput(0, readerIma->GetOutput());
  filter->SetInput(1, readerMask->GetOutput());
  filter->GetFunctor().SetDates(dates);

  auto writer = otb::ImageFileWriter<ImageType>::New();
  writer->SetFileName(argv[4]);
  writer->SetInput(filter->GetOutput());

  otb::StandardFilterWatcher watcher(writer, "SigmoFitting");
  writer->Update();

  return EXIT_SUCCESS;
}



