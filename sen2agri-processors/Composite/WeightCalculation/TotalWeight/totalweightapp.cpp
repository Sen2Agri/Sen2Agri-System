
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

#include "totalweightcomputation.h"

namespace otb
{

namespace Wrapper
{

class TotalWeightApp : public Application
{

public:
  /** Standard class typedefs. */
  typedef TotalWeightApp                      Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(TotalWeightApp, otb::Application);

private:
  void DoInit()
  {
    SetName("TotalWeight");
    SetDescription("Computes the cloud weight value for the given mask cloud and parameters");

    SetDocName("Total Weight Computation");
    SetDocLongDescription("This application computes the total weight for the Composite Processor");

    SetDocLimitations("None");
    SetDocAuthors("CIU");
    SetDocSeeAlso(" ");
    AddDocTag("Util");

    AddParameter(ParameterType_String,  "in",   "Input product file name");
    SetParameterDescription("in", "The product file name used to determine the sensor type.");

    AddParameter(ParameterType_Float, "wsensor", "Weight for the given sensor type");
    SetParameterDescription("wsensor", "The weight to be used for the given sensor type.");

    AddParameter(ParameterType_Int, "l2adate", "L2A date, expressed in days");
    SetParameterDescription("l2adate", "The L2A date extracted from metadata, expressed in days.");

    AddParameter(ParameterType_Int, "l3adate", "L3A date, expressed in days");
    SetParameterDescription("l3adate", "The L3A date extracted from metadata, expressed in days.");

    AddParameter(ParameterType_Int, "halfsynthesis", "Delta max");
    SetParameterDescription("halfsynthesis", "Half synthesis period expressed in days.");

    AddParameter(ParameterType_Float, "wdatemin", "Minimum date weight");
    SetParameterDescription("wdatemin", "Minimum weight at edge of synthesis time window.");
    SetDefaultParameterFloat("wdatemin", 0.5);

    AddParameter(ParameterType_String,  "waotfile",   "Input AOT weight file name");
    SetParameterDescription("waotfile", "The file name of the image containing the AOT weigth for each pixel.");

    AddParameter(ParameterType_String,  "wcldfile",   "Input cloud weight file name");
    SetParameterDescription("wcldfile", "The file name of the image containing the cloud weigth for each pixel.");

    AddParameter(ParameterType_OutputImage, "outtotalweight", "Output Total Weight Image");
    SetParameterDescription("outtotalweight","The output image containg the computed total weight for each pixel.");

    // Doc example parameter settings
    SetDocExampleParameterValue("in", "example1.tif");
    SetDocExampleParameterValue("waotfile", "example2.tif");
    SetDocExampleParameterValue("wcldfile", "example3.tif");
    SetDocExampleParameterValue("wsensor", "0.33");
    SetDocExampleParameterValue("l2adate", "10");
    SetDocExampleParameterValue("l3adate", "20");
    SetDocExampleParameterValue("halfsynthesis", "50");
    SetDocExampleParameterValue("wdatemin", "0.10");
    SetDocExampleParameterValue("outtotalweight", "apTotalWeightOutput.tif");
  }

  void DoUpdateParameters()
  {
  }

  void DoExecute()
  {
    // Get the input image list
    std::string inProdFileName = GetParameterString("in");
    if (inProdFileName.empty())
    {
        itkExceptionMacro("No input Image set...; please set the input image");
    }
    float weightSensor = GetParameterFloat("wsensor");
    int l2aDate = GetParameterInt("l2adate");
    int l3aDate = GetParameterInt("l3adate");
    int halfSynthesis = GetParameterInt("halfsynthesis");
    float weightOnDateMin = GetParameterFloat("wdatemin");

    std::string inAotFileName = GetParameterString("waotfile");
    std::string inCloudFileName = GetParameterString("wcldfile");

    // weight on sensor parameters
    m_totalWeightComputation.SetInputProductName(inProdFileName);
    m_totalWeightComputation.SetWeightOnSensor(weightSensor);

    // weight on date parameters
    m_totalWeightComputation.SetL2ADateAsDays(l2aDate);
    m_totalWeightComputation.SetL3ADateAsDays(l3aDate);
    m_totalWeightComputation.SetHalfSynthesisPeriodAsDays(halfSynthesis);
    m_totalWeightComputation.SetWeightOnDateMin(weightOnDateMin);


    // Weights for AOT and Clouds
    m_totalWeightComputation.SetAotWeightFile(inAotFileName);
    m_totalWeightComputation.SetCloudsWeightFile(inCloudFileName);

    // The output file name
    //m_totalWeightComputation.SetTotalWeightOutputFileName();


    // Set the output image
    SetParameterOutputImage("out_total_weight", m_totalWeightComputation.GetOutputImageSource()->GetOutput());
  }

  TotalWeightComputation m_totalWeightComputation;
};

} // namespace Wrapper
} // namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::TotalWeightApp)

