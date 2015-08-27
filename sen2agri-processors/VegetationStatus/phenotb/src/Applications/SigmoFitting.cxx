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

namespace otb
{
namespace Wrapper
{

class SigmoFitting : public Application
{
public:
/** Standard class typedefs. */
  typedef SigmoFitting     Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  
/** Standard macro */
  itkNewMacro(Self);
  itkTypeMacro(SigmoFitting, otb::Application);

  using FunctorType =
    pheno::TwoCycleSigmoFittingFunctor<FloatVectorImageType::PixelType>;
  using FilterType = pheno::BinaryFunctorImageFilterWithNBands<FloatVectorImageType,
                                                               FloatVectorImageType,
                                                               FunctorType>;

private:
  void DoInit()
  {
    SetName("SigmoFitting");
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

  virtual ~SigmoFitting()
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
    FloatVectorImageType::Pointer maskImage;
    bool use_mask = true;
    if(IsParameterEnabled("mask"))
      maskImage = this->GetParameterImage("mask");
    else
      {
      maskImage = inputImage;
      use_mask = false;
      otbAppLogINFO("No mask will be used.\n");
      }
    inputImage->UpdateOutputInformation();
    maskImage->UpdateOutputInformation();
    bool fit_mode = true;
    unsigned int nb_out_bands = inputImage->GetNumberOfComponentsPerPixel();
    if(IsParameterEnabled("mode") && GetParameterString("mode") == "params")
      {
      fit_mode = false;
      nb_out_bands = 12;
      otbAppLogINFO("Parameter estimation mode.\n");
      }

    filter = FilterType::New();

    filter->SetInput(0, inputImage);
    filter->SetInput(1, maskImage);
    filter->GetFunctor().SetDates(dates);
    filter->GetFunctor().SetUseMask(use_mask);
    filter->GetFunctor().SetReturnFit(fit_mode);
    filter->SetNumberOfOutputBands(nb_out_bands);
    filter->UpdateOutputInformation();
    SetParameterOutputImage("out", filter->GetOutput());
  }

  FilterType::Pointer filter;

};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::SigmoFitting)
