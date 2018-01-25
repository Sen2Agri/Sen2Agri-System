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

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbWrapperChoiceParameter.h"

#include "phenoFunctions.h"
#include "GlobalDefs.h"

// we have 4 phenological parameters and 1 band for the flags
#define RESULT_BANDS_NO     5
namespace pheno
{
template <typename PixelType>
class PhenologicalNDVIMetricsFunctor
{
protected:
  VectorType dv;

public:
  struct DifferentSizes {};
  PhenologicalNDVIMetricsFunctor() {};

  void SetDates(const std::vector<tm>& d) {
    dv = VectorType{static_cast<unsigned int>(d.size())};
    const auto &days = pheno::tm_to_doy_list(d);
    std::copy(std::begin(days), std::end(days), std::begin(dv));
  }

  // dates is already given in doy
  // valid pixel has a mask==0
  PixelType operator()(PixelType pix, PixelType mask)
  {
    auto nbDates = pix.GetSize();

    if(dv.size()!=nbDates) throw DifferentSizes{};

    PixelType tmpix{nbDates};
    tmpix.Fill(typename PixelType::ValueType{0});

    VectorType vec(nbDates);
    for(size_t i=0; i<nbDates; i++)
    {
      vec[i] = pix[i];
    }

    // A date is valid if it is not NaN and the mask value == 0.
    auto pred = [=](int e) { return !(std::isnan(vec[e])) && (vec[e] >= 0) &&
                             (mask[e]==(typename PixelType::ValueType{IMG_FLG_LAND})); };
    auto f_profiles = filter_profile_fast(vec, dv, pred);

    decltype(vec) profile=f_profiles.first;
    decltype(vec) t=f_profiles.second;

    // If there are not enough valid dates, keep the original value
    PixelType result(RESULT_BANDS_NO);
    result.Fill(0);
    if(profile.size() < 4) {
      return result;
    }

    auto approx = normalized_sigmoid::TwoCycleApproximation(profile, t);
    auto princ_cycle = std::get<1>(approx);
    auto x_hat = std::get<0>(princ_cycle);
    auto min_max = std::get<1>(princ_cycle);
    auto A_hat = min_max.second - min_max.first;
    auto B_hat = min_max.first;

     double dgx0, t0, t1, t2, t3, dgx2;
     std::tie(dgx0, t0, t1, t2, t3, dgx2) =
         pheno::normalized_sigmoid::pheno_metrics<double>(x_hat, A_hat, B_hat);

    // add also the number of valid dates that were used in the processing
    // The metrics have to fulfill some constraints in order to be considered as valid:
    //    t0<x0<t1<t2<t3
    if((0 <= t0) && (t0 < x_hat[0]) && (x_hat[0] < t1) && (t1 < t2) && (t2 < t3) && ((t3 - t0) < 365)) {
        result[0] = x_hat[0];
        result[1] = t0;
        result[2] = (t2-t1);
        result[3] = t3;
        result[4] = profile.size();
    }

    return result;
  }

  bool operator!=(const PhenologicalNDVIMetricsFunctor a)
  {
    return (this->dates != a.dates) || (this->dv != a.dv) ;
  }

  bool operator==(const PhenologicalNDVIMetricsFunctor a)
  {
    return !(*this == a);
  }

};
}

namespace otb
{
namespace Wrapper
{

class PhenologicalNDVIMetrics : public Application
{
public:
/** Standard class typedefs. */
  typedef PhenologicalNDVIMetrics     Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  
/** Standard macro */
  itkNewMacro(Self);
  itkTypeMacro(PhenologicalNDVIMetrics, otb::Application);

  using FunctorType =
    pheno::PhenologicalNDVIMetricsFunctor<FloatVectorImageType::PixelType>;
  using FilterType = pheno::BinaryFunctorImageFilterWithNBands<FloatVectorImageType,
                                                               FloatVectorImageType,
                                                               FunctorType>;


private:
  void DoInit()
  {
    SetName("PhenologicalNDVIMetrics");
    SetDescription("");

    // Documentation
    SetDocName("Double sigmoid fitting");
    SetDocLongDescription("");
    SetDocLimitations("None");
    SetDocAuthors("Jordi Inglada");
    
    AddParameter(ParameterType_InputImage,  "in",   "Input Image");
    SetParameterDescription("in", "Input image");
    MandatoryOn("in");

    AddParameter(ParameterType_InputImage,  "mask",   "Mask Image");
    SetParameterDescription("mask", "Input validity mask");
    MandatoryOff("mask");

    AddParameter(ParameterType_String, "dates", "Date File");
    SetParameterDescription("dates", "Name of the file containing the dates for the images");
    MandatoryOn("dates");

    AddParameter(ParameterType_String, "mode", "Processing mode: profile or params");
    SetParameterDescription("mode", "Choose between producing as output the reprocessed profile or the parametres of the two-cycle sigmoid. Default value is profile.");
    MandatoryOff("mode");

    AddParameter(ParameterType_OutputImage, "out",  "Output Image");
    SetParameterDescription("out", "Output image");
    MandatoryOn("out");
    
    AddRAMParameter();
  }

  virtual ~PhenologicalNDVIMetrics()
  {
  }


  void DoUpdateParameters() 
  {
    // Nothing to do here : all parameters are independent
  }


  void DoExecute() 
  {

    // prepare the vector of dates
    auto dates = pheno::parse_date_file(this->GetParameterString("dates"));
    // pipeline
    FloatVectorImageType::Pointer inputImage = this->GetParameterImage("in");
    FloatVectorImageType::Pointer maskImage = this->GetParameterImage("mask");
    inputImage->UpdateOutputInformation();
    maskImage->UpdateOutputInformation();
    filter = FilterType::New();

    filter->SetInput(0, inputImage);
    filter->SetInput(1, maskImage);
    filter->GetFunctor().SetDates(dates);
    filter->SetNumberOfOutputBands(RESULT_BANDS_NO);
    filter->UpdateOutputInformation();
    // the pheno ndvi metrics are already int values as they are only dates
    SetParameterOutputImage("out", filter->GetOutput());
    SetParameterOutputImagePixelType("out", ImagePixelType_int16);
  }

  FilterType::Pointer filter;

};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::PhenologicalNDVIMetrics)
