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
#include "dateUtils.h"
#include "phenoFunctions.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include <algorithm>

int main(int argc, char* argv[])
{
  if(argc!=7)
    {
    itkGenericExceptionMacro(<< "Usage: " << argv[0] << " imageFileName datesFileName year nodataValue line col\n");
    }

  using Reader = otb::ImageFileReader<otb::VectorImage<double,2>>;
  Reader::Pointer reader = Reader::New();
  reader->SetFileName(argv[1]);
  reader->Update();

  auto date_vec = pheno::parse_date_file(argv[2]);
  itk::Index<2> coords{std::stoi(argv[5]), std::stoi(argv[6])};
  auto pix = pheno::pixelToVector(reader->GetOutput()->GetPixel(coords));

  auto noData = std::stod(argv[4]);
  auto year = std::stoi(argv[3]);

  // The selection predicate using the year and no-data
  auto pred = [=](int e){return (pix[e]!=noData && date_vec[e].tm_year==year);};

  unsigned int nbDates{0};
  for(size_t i=0; i<date_vec.size(); i++) if(pred(i)) nbDates++;

  decltype(pix) vpix(nbDates), vdates(nbDates);

  auto dcount = 0;

  for(size_t i=0; i<date_vec.size(); i++)
    if(pred(i))
      {
      vpix[dcount] = pix[i];
      vdates[dcount] = pheno::doy(date_vec[i]);
      dcount++;
      }


  auto minmax = std::minmax_element(std::begin(vpix), std::end(vpix));
  const unsigned int radius{2};


  auto pradius = 3;
  auto pmax = minmax.second - std::begin(vpix);
  if(pmax < 4)
    pradius = 2;


  using FilterType = pheno::FilterType<typename pheno::VectorType>;
  FilterType med = pheno::getMedian<typename pheno::VectorType>;
  FilterType lp = pheno::getLocalParabola<typename pheno::VectorType>;

  auto res = pheno::profileFilter(pheno::profileFilter(vpix,
                                                       vdates, med, radius),
                                  vdates, lp, pradius);


  for(size_t i=0; i<nbDates; i++)
    std::cout << vdates[i] << "\t" << vpix[i] << "\t" << res[i] << std::endl;

  return 0;
}
