
/*=========================================================================

 Program:   ORFEO Toolbox
 Language:  C++
 Date:      $Date$
 Version:   $Revision$


 Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
 See OTBCopyright.txt for details.


 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbObjectList.h"

namespace otb
{

namespace Wrapper
{

class ComputeTotalWeight : public Application
{

public:
  /** Standard class typedefs. */
  typedef ComputeTotalWeight            Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(ComputeTotalWeight, otb::Application);

  typedef otb::MultiToMonoChannelExtractROI<FloatVectorImageType::InternalPixelType,
                                            FloatImageType::PixelType>    ExtractROIFilterType;
  typedef otb::ObjectList<ExtractROIFilterType>                           ExtractROIFilterListType;
  typedef otb::BandMathImageFilter<FloatImageType>                             BandMathImageFilterType;


private:
  void DoInit()
  {
    SetName("ComputeTotalWeight");
    SetDescription("Perform a mathematical operation on monoband images");

    SetDocName("Weight AOT");
    SetDocLongDescription("TODO.");

    SetDocLimitations("None");
    SetDocAuthors("CIU");
    SetDocSeeAlso(" ");
    AddDocTag("Util");

    AddParameter(ParameterType_InputImage,  "in",   "Input image");
    SetParameterDescription("in", "Image containing AOT.");

    AddParameter(ParameterType_String, "band", "Expression");
    SetParameterDescription("band", "The AOT band from the input image.");

    AddParameter(ParameterType_OutputImage, "out", "Output Image");
    SetParameterDescription("out","Output image.");

    AddRAMParameter();

    AddParameter(ParameterType_Float, "qaot", "AOTQuantificationValue");
    SetParameterDescription("qaot", "AOT quantification value");

    AddParameter(ParameterType_Float, "waotmin", "WeightAOTMin");
    SetParameterDescription("waotmin", "min weight depending on AOT");

    AddParameter(ParameterType_Float, "waotmax", "WeightAOTMax");
    SetParameterDescription("waotmax", "max weight depending on AOT");

    AddParameter(ParameterType_Float, "aotmax", "AOTMax");
    SetParameterDescription("aotmax",
        "maximum value of the linear range for weights w.r.t AOT to which the quantification value is applied");

    // Doc example parameter settings
    SetDocExampleParameterValue("in", "verySmallFSATSW_r.tif");
    SetDocExampleParameterValue("qaot", "0.005");
    SetDocExampleParameterValue("waotmin", "0.33");
    SetDocExampleParameterValue("waotmax", "1");
    SetDocExampleParameterValue("aotmax", "50");
    SetDocExampleParameterValue("out", "apAOTWeightOutput.tif");
  }

  void DoUpdateParameters()
  {
  }

  void DoExecute()
  {
/*  TODO

    // Get the input image list
    FloatVectorImageType::Pointer inImg = GetParameterImage("in");

    if (inImg.IsNull())
      {
       itkExceptionMacro("No input Image set...; please set the input image");
      }

    m_ChannelExtractorList = ExtractROIFilterListType::New();
    m_Filter               = BandMathImageFilterType::New();

    inImg->UpdateOutputInformation();

    otbAppLogINFO( << "Input Image has " << inImg->GetNumberOfComponentsPerPixel()
                     << " components" << std::endl );

    for (unsigned int j = 0; j < inImg->GetNumberOfComponentsPerPixel(); j++)
    {
        std::ostringstream tmpParserVarName;
        tmpParserVarName << "im1" << "b" << j + 1;

        m_ExtractROIFilter = ExtractROIFilterType::New();
        m_ExtractROIFilter->SetInput(inImg);
        m_ExtractROIFilter->SetChannel(j + 1);
        m_ExtractROIFilter->GetOutput()->UpdateOutputInformation();
        m_ChannelExtractorList->PushBack(m_ExtractROIFilter);
        m_Filter->SetNthInput(0, m_ChannelExtractorList->Back()->GetOutput(), tmpParserVarName.str());
    }

    std::string strBand = GetParameterString("band");
    float fAotQuantificationVal = GetParameterFloat("qaot");
    float fAotMax = GetParameterFloat("aotmax");
    float fWaotMin = GetParameterFloat("waotmin");
    float fWaotMax = GetParameterFloat("waotmax");


    std::ostringstream exprStream;
    exprStream << "(im1" << strBand << "/" << fAotQuantificationVal << "<="
               << fAotMax << "/" << fAotQuantificationVal << ") ? "
               <<  fWaotMin + (fWaotMax-fWaotMin) << " * (1-" << "im1" << strBand << "/" <<
               fAotQuantificationVal << "/" << fAotMax << ")";

    otbAppLogINFO( << "Expression used for AOT: " << exprStream.str() << std::endl );

    m_Filter->SetExpression(exprStream.str());

    // Set the output image
    SetParameterOutputImage("out", m_Filter->GetOutput());
*/
  }

  ExtractROIFilterType::Pointer     m_ExtractROIFilter;
  ExtractROIFilterListType::Pointer m_ChannelExtractorList;
  BandMathImageFilterType::Pointer  m_Filter;
};

} // namespace Wrapper
} // namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::ComputeTotalWeight)


