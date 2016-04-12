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
#include "phenoFunctions.h"
#include "dateUtils.h"
#include "miscUtils.h"

#include "otbVectorImage.h"
#include "otbImageFileReader.h"

#include <cmath>

// export ITK_AUTOLOAD_PATH="" && cd ~/Dev/builds/phenotb/ && make && ~/Dev/builds/phenotb/Applications/explore-lai ~/stok/DATA/LAI_F2_Aurade/LAI_F2_Aurade.tif ~/stok/DATA/LAI_F2_Aurade/LAI_F2_Aurade_dates.txt 2006 -32768 127 229


namespace pheno {




}

int main(int argc, char* argv[])
{

  if(argc!=5 && argc!=7)
    {
    itkGenericExceptionMacro(<< "Usage: " << argv[0] << " imageFileName datesFileName year nodataValue [line] [col]\n");
    }

  using Reader = otb::ImageFileReader<otb::VectorImage<double,2>>;
  Reader::Pointer reader = Reader::New();
  reader->SetFileName(argv[1]);
  reader->Update();

  auto imageSize = (reader->GetOutput()->GetLargestPossibleRegion()).GetSize();

  std::cout << "Image size is " << imageSize << std::endl;

  auto date_vec = pheno::parse_date_file(argv[2]);

  long int fewdatescount{0};
  long int goodcount{0};
  long int ngcount{0};
  long int nancount{0};
  long int lowcount{0};
  for(size_t xc=1; xc<imageSize[0]-1; xc++)
    {
    for(size_t yc=1; yc<imageSize[1]-1; yc++)
      {
      itk::Index<2> coords{static_cast<long int>(xc), 
          static_cast<long int>(yc)};

      if(argc==7)
        {
        coords[0] = std::stoi(argv[5]);
        coords[1] = std::stoi(argv[6]);
        }

      std::cout << coords[0] << "," << coords[1] << " | ";
      
      auto vec = pheno::pixelToVector(reader->GetOutput()->GetPixel(coords));
      auto noData = std::stod(argv[4]);
      auto year = std::stoi(argv[3]);

      // The selection predicate using the year and no-data
      auto pred = [=](int e){return (vec[e]!=noData && date_vec[e].tm_year==year);};

      auto f_profiles = pheno::filter_profile(vec, date_vec, pred);

      decltype(vec) profile=f_profiles.first;
      decltype(vec) t=f_profiles.second;
      // We have to count the valid dates since vnl_vector has fixed size
      unsigned int nbDates{t.size()};
      unsigned int params{4};
      if(nbDates<params)
        {
        std::cout << "No dates for pixel " << coords[0] << "," << coords[1] << std::endl;
        fewdatescount++;
        continue;
        }

      // begin of the approximation


      auto minmax = std::minmax_element(std::begin(profile), std::end(profile));

      auto vmax = *(minmax.second);
      auto vmin = *(minmax.first);

      if(vmax < 1000)
        {
        lowcount++;
        continue;
        }
      
      // normalise the profile
      profile = (profile-vmin)/(vmax-vmin);
      
      auto pradius = 3;
      auto pmax = minmax.second - std::begin(profile);
      if(pmax < 4)
        pradius = 2;


      using FilterType = pheno::FilterType<typename pheno::VectorType>;
      FilterType med = pheno::getMedian<typename pheno::VectorType>;
      FilterType lp = pheno::getLocalParabola<typename pheno::VectorType>;
      auto x(pheno::normalized_sigmoid::guesstimator(
               (
                 (nbDates<14)?profile:
                 pheno::profileFilter(
                   (pheno::profileFilter(profile, t, med, 2)),
                   t, lp, pradius)),
               t));
      std::cout << x << std::endl;


      auto fprofile = pheno::gaussianWeighting(profile, t);
      pheno::normalized_sigmoid::F<pheno::VectorType> phFs;
      pheno::ParameterCostFunction<pheno::normalized_sigmoid::F<pheno::VectorType>> fs{params, nbDates, fprofile, t, phFs};

      auto err(pheno::optimize(x, fs));


      // error diagnostics

      auto error_code = pheno::normalized_sigmoid::error_diagnostics(x, profile, t);         
      
      // error per number of point data
      err /= nbDates;

      std::cout << x << " --> " << err << std::endl;

           
      if(error_code!=pheno::FittingErrors::HasNan)
        {
        if(nbDates < params)
          {
          fewdatescount++;
          }
        else
          {
          pheno::VectorType yHat(nbDates);
          phFs(t,x,yHat);

          // rerscale the values with min and max
          yHat = yHat*(vmax-vmin)+vmin;
          profile = profile*(vmax-vmin)+vmin;
      
          std::stringstream fnss;
          std::string ssng = (error_code==pheno::FittingErrors::NOK)?"-NG":"";
          fnss << "/tmp/test-" << coords[0] << "-" << coords[1] << ssng << ".gp";

          std::stringstream title;
          title << "(" << coords[0] << "," << coords[1] << ") : ";

          for(auto & elem : x)
            {
            title << elem << " ";
            }
          title << "| e=" << err;
          pheno::generatePlotFile(fnss.str(), title.str(), t, profile, yHat, x);
          if(error_code==pheno::FittingErrors::NOK) ngcount++;
          else goodcount++;
          }
        }
      else nancount++;

      if(argc==7) break;
      }
    if(argc==7) break;
    }
  std::cout << goodcount << " good, " << lowcount << " low LAI, " << fewdatescount << " few dates, " << ngcount << " bad (" << nancount << " nan)" << std::endl;
  return 0;
}
