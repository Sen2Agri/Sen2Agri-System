/*=========================================================================
  Program:   otb-bv
  Language:  C++

  Copyright (c) CESBIO. All rights reserved.

  See otb-bv-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "itkBinaryFunctorImageFilter.h"

#include <vector>

#include "dateUtils.h"
#include "phenoFunctions.h"
#include "otbProfileReprocessing.h"
#include "MetadataHelperFactory.h"

namespace otb
{
int date_to_doy(std::string& date_str)
{
  return pheno::doy(pheno::make_date(date_str));
}

namespace Functor
{
}


namespace Wrapper
{

typedef enum {ALGO_LOCAL = 0, ALGO_FIT} ALGO_TYPE;
template< class TInput1, class TInput2, class TOutput>
class ProfileReprocessingFunctor
{
public:
  ProfileReprocessingFunctor() {}
  ~ProfileReprocessingFunctor() {}
  bool operator!=(const ProfileReprocessingFunctor &a) const
  {
      return (this->date_vect != a.date_vect) || (this->algoType != a.algoType) ||
              (this->bwr != a.bwr) || (this->fwr != a.fwr);
  }

  bool operator==(const ProfileReprocessingFunctor & other) const
  {
    return !( *this != other );
  }

  void SetDates(VectorType &idDates)
  {
      date_vect = idDates;
  }

  void SetAlgoType(ALGO_TYPE algo)
  {
      algoType = algo;
  }

  void SetBwr(size_t inBwr) {
      bwr = inBwr;
  }

  void SetFwr(size_t inFwr) {
      fwr = inFwr;
  }

  inline TOutput operator()(const TInput1 & A,
                            const TInput2 & B) const
  {
      //itk::VariableLengthVector vect;
    int nbBvElems = A.GetNumberOfElements();

    VectorType ts(nbBvElems);
    VectorType ets(nbBvElems);
    int i;
    for(i = 0; i<nbBvElems; i++) {
        ts[i] = A[i];
        ets[i] = B[i];
    }

    VectorType out_bv_vec{};
    VectorType out_flag_vec{};

    if(algoType == ALGO_LOCAL) {
        std::tie(out_bv_vec, out_flag_vec) =
            smooth_time_series_local_window_with_error(date_vect, ts, ets,
                                                       bwr, fwr);
    } else {
      std::tie(out_bv_vec, out_flag_vec) =
        fit_csdm(date_vect, ts, ets);
    }
    TOutput result(2*nbBvElems);
    i = 0;
    for(i = 0; i < nbBvElems; i++) {
        result[i] = out_bv_vec[i];
    }
    for(int j = 0; j < nbBvElems; j++) {
        result[i+j] = out_flag_vec[j];
    }

    return result;
  }
private:
  // input dates vector
  VectorType date_vect;
  ALGO_TYPE algoType;
  size_t bwr;
  size_t fwr;
};

class ProfileReprocessing : public Application
{
public:
  /** Standard class typedefs. */
  typedef ProfileReprocessing               Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  typedef float                                   PixelType;
  typedef FloatVectorImageType                    InputImageType;
  typedef FloatVectorImageType                    OutImageType;

  typedef ProfileReprocessingFunctor <InputImageType::PixelType,
                                    InputImageType::PixelType,
                                    OutImageType::PixelType>                FunctorType;

  typedef itk::BinaryFunctorImageFilter<InputImageType,
                                        InputImageType,
                                        OutImageType, FunctorType> FilterType;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(ProfileReprocessing, otb::Application);

private:
  void DoInit()
  {

    SetName("ProfileReprocessing");
    SetDescription("Reprocess a BV time profile.");
   
    AddParameter(ParameterType_InputImage, "lai", "Input profile file.");
    SetParameterDescription( "lai", "Input file containing the profile to process. This is an ASCII file where each line contains the date (YYYMMDD) the BV estimation and the error." );

    AddParameter(ParameterType_InputImage, "err", "Input profile file.");
    SetParameterDescription( "err", "Input file containing the profile to process. This is an ASCII file where each line contains the date (YYYMMDD) the BV estimation and the error." );

    AddParameter(ParameterType_InputFilenameList, "ilxml", "The XML metadata files list");

    AddParameter(ParameterType_OutputImage, "opf", "Output profile file.");
    SetParameterDescription( "opf", "Filename where the reprocessed profile saved. This is an raster band contains the new BV estimation value for each pixel. The last band contains the boolean information which is 0 if the value has not been reprocessed." );

    AddParameter(ParameterType_Choice, "algo", 
                 "Reprocessing algorithm: local, fit.");
    SetParameterDescription("algo", 
                            "Reprocessing algorithm: local uses a window around the current date, fit is a double logisting fitting of the complete profile.");

    AddChoice("algo.fit", "Double logistic fitting of the complete profile.");
    SetParameterDescription("algo.fit", "This group of parameters allows to set fit window parameters. ");

    AddChoice("algo.local", "Uses a window around the current date.");
    SetParameterDescription("algo.local", "This group of parameters allows to set local window parameters. ");

    AddParameter(ParameterType_Int, "algo.local.bwr", "Local window backward radius");
    SetParameterInt("algo.local.bwr", 1);
    SetParameterDescription("algo.local.bwr", "Backward radius of the local window. ");

    AddParameter(ParameterType_Int, "algo.local.fwr", "Local window forward radius");
    SetParameterInt("algo.local.fwr", 1);
    SetParameterDescription("algo.local.fwr", "Forward radius of the local window. ");

    MandatoryOff("algo");
  }

  void DoUpdateParameters()
  {
    //std::cout << "ProfileReprocessing::DoUpdateParameters" << std::endl;
  }

  void DoExecute()
  {
      FloatVectorImageType::Pointer lai_image = this->GetParameterImage("lai");
      FloatVectorImageType::Pointer err_image = this->GetParameterImage("err");
      std::vector<std::string> xmlsList = this->GetParameterStringList("ilxml");
      unsigned int nb_lai_bands = lai_image->GetNumberOfComponentsPerPixel();
      unsigned int nb_err_bands = err_image->GetNumberOfComponentsPerPixel();
      unsigned int nb_xmls = xmlsList.size();
      if((nb_lai_bands == 0) || (nb_lai_bands != nb_err_bands) || (nb_lai_bands != nb_xmls)) {
          itkExceptionMacro("Invalid number of bands or xmls: lai bands=" <<
                            nb_lai_bands << ", err bands =" <<
                            nb_err_bands << ", nb_xmls=" << nb_xmls);
      }

      std::string date_str;
      std::vector<std::tm> dv;
      VectorType inDates;
      for (std::string strXml : xmlsList)
      {
          MetadataHelperFactory::Pointer factory = MetadataHelperFactory::New();
          // we are interested only in the 10m resolution as we need only the date
          auto pHelper = factory->GetMetadataHelper(strXml, 10);
          date_str = pHelper->GetAcquisitionDate();
          //inDates.push_back(date_to_doy(date_str));
          struct tm tmDate = {};
          if (strptime(date_str.c_str(), "%Y%m%d", &tmDate) == NULL) {
              itkExceptionMacro("Invalid value for a date: " + date_str);
          }
          dv.push_back(tmDate);

      }
      auto times = pheno::tm_to_doy_list(dv);
      inDates = VectorType(dv.size());
      std::copy(std::begin(times), std::end(times), std::begin(inDates));

      size_t bwr{1};
      size_t fwr{1};
      ALGO_TYPE algoType = ALGO_LOCAL;
      std::string algo{"local"};
      if (IsParameterEnabled("algo"))
        algo = GetParameterString("algo");
      if (algo == "local")
      {
        if (IsParameterEnabled("algo.local.bwr"))
          bwr = GetParameterInt("algo.local.bwr");
        if (IsParameterEnabled("algo.local.fwr"))
          bwr = GetParameterInt("algo.local.fwr");
      } else {
          algoType = ALGO_FIT;
      }

      //instantiate a functor with the regressor and pass it to the
      //unary functor image filter pass also the normalization values
      m_profileReprocessingFilter = FilterType::New();
      m_functor.SetDates(inDates);
      m_functor.SetAlgoType(algoType);
      m_functor.SetBwr(bwr);
      m_functor.SetFwr(fwr);

      m_profileReprocessingFilter->SetFunctor(m_functor);
      m_profileReprocessingFilter->SetInput1(lai_image);
      m_profileReprocessingFilter->SetInput2(err_image);
      m_profileReprocessingFilter->UpdateOutputInformation();
      m_profileReprocessingFilter->GetOutput()->SetNumberOfComponentsPerPixel(nb_lai_bands*2);
      SetParameterOutputImage("opf", m_profileReprocessingFilter->GetOutput());

  }

  FilterType::Pointer m_profileReprocessingFilter;
  FunctorType m_functor;
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::ProfileReprocessing)
