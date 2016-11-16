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

#include "otbChangeLabelImageFilter.h"
#include "otbStandardWriterWatcher.h"
#include "otbStatisticsXMLFileReader.h"
#include "otbShiftScaleVectorImageFilter.h"
#include "otbMultiModelImageClassificationFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbMachineLearningModelFactory.h"
#include "otbObjectList.h"

namespace otb
{
namespace Wrapper
{

class MultiModelImageClassifier : public Application
{
public:
  /** Standard class typedefs. */
  typedef MultiModelImageClassifier     Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Standard macro */
  itkNewMacro(Self);

  itkTypeMacro(MultiModelImageClassifier, otb::Application);

  /** Filters typedef */
  typedef UInt16ImageType                                                                                OutputImageType;
  typedef UInt8ImageType                                                                                 MaskImageType;
  typedef itk::VariableLengthVector<FloatVectorImageType::InternalPixelType>                             MeasurementType;
  typedef otb::StatisticsXMLFileReader<MeasurementType>                                                  StatisticsReader;
  typedef otb::ShiftScaleVectorImageFilter<FloatVectorImageType, FloatVectorImageType>                   RescalerType;
  typedef otb::MultiModelImageClassificationFilter<FloatVectorImageType, OutputImageType, MaskImageType> ClassificationFilterType;
  typedef ClassificationFilterType::ModelType                                                            ModelType;
  typedef otb::ObjectList<ModelType>                                                                     ModelListType;
  typedef otb::ObjectList<RescalerType>                                                                  RescalerListType;
  typedef ClassificationFilterType::ValueType                                                            ValueType;
  typedef ClassificationFilterType::LabelType                                                            LabelType;
  typedef otb::MachineLearningModelFactory<ValueType, LabelType>                                         MachineLearningModelFactoryType;

private:
  void DoInit()
  {
    SetName("MultiModelImageClassifier");
    SetDescription("Performs a classification of the input image according to multiple model files corresponding to different regions of the image.");

    // Documentation
    SetDocName("Image Classification");
    SetDocLongDescription("This application performs an image classification based on model files produced by the TrainImagesClassifier application. Pixels of the output image will contain the class labels decided by the classifier (maximal class label = 65535). The input pixels can be optionally centered and reduced according to the statistics file produced by the ComputeImagesStatistics application. An optional model mask can be provided, which specifies which of the loaded models to use when classyfing each pixel. A model mask pixel of 0 means that the input pixel should not be classified.");

    SetDocLimitations("The input image must have the same type, order and number of bands than the images used to produce the statistics file and the SVM model file. If a statistics file was used during training by the TrainImagesClassifier, it is mandatory to use the same statistics file for classification. If an input mask is used, its size must match the input image size.");
    SetDocAuthors("Laurentiu Nicola");
    SetDocSeeAlso("TrainImagesClassifier, ValidateImagesClassifier, ComputeImagesStatistics");

    AddDocTag(Tags::Learning);

    AddParameter(ParameterType_InputImageList, "in",  "Input Images");
    SetParameterDescription( "in", "The input images (one for each model) to classify.");

    AddParameter(ParameterType_InputImage,  "mask",   "Input model Mask");
    SetParameterDescription( "mask", "The mask specifies which model applies to an input image pixel.");
    MandatoryOff("mask");

    AddParameter(ParameterType_InputFilenameList, "model", "Model files");
    SetParameterDescription("model", "One or more model files (produced by TrainImagesClassifier application, maximal class label = 65535).");

    AddParameter(ParameterType_StringList, "modelid", "Model identifiers");
    SetParameterDescription("modelid", "Model identifiers");
    MandatoryOff("modelid");

    AddParameter(ParameterType_Int, "nodatalabel", "No data label");
    SetDefaultParameterInt("nodatalabel", 0);
    SetParameterDescription("nodatalabel", "The label to output for masked pixels.");

    AddParameter(ParameterType_InputFilenameList, "imstat", "Statistics file");
    SetParameterDescription("imstat", "One XML file per model containing mean and standard deviation to center and reduce samples before classification (produced by ComputeImagesStatistics application).");
    MandatoryOff("imstat");

    AddParameter(ParameterType_OutputImage, "out",  "Output Image");
    SetParameterDescription( "out", "Output image containing class labels");
    SetParameterOutputImagePixelType( "out", ImagePixelType_uint16);

    AddRAMParameter();

   // Doc example parameter settings
    SetDocExampleParameterValue("in", "QB_1_ortho.tif");
    SetDocExampleParameterValue("imstat", "EstimateImageStatisticsQB1.xml");
    SetDocExampleParameterValue("model", "clsvmModelQB1.svm");
    SetDocExampleParameterValue("nodatalabel", "-10000");
    SetDocExampleParameterValue("out", "clLabeledImageQB1.tif");
  }

  void DoUpdateParameters()
  {
    // Nothing to do here : all parameters are independent
  }

  void DoExecute()
  {
    // Load input image
    FloatVectorImageListType::Pointer inImages = GetParameterImageList("in");
    inImages->UpdateOutputInformation();

    // Load models
    otbAppLogINFO("Loading models");

    m_Models = ModelListType::New();
    const std::vector<std::string> &modelFiles = GetParameterStringList("model");
    m_Models->Reserve(modelFiles.size());

    for (std::vector<std::string>::const_iterator it = modelFiles.begin(), itEnd = modelFiles.end(); it != itEnd; ++it)
      {
      ModelType::Pointer model = MachineLearningModelFactoryType::CreateMachineLearningModel(*it,
                                                                            MachineLearningModelFactoryType::ReadMode);

      if (model.IsNull())
        {
        otbAppLogFATAL(<< "Error when loading model " << *it << " : unsupported model type");
        }

      model->Load(*it);

      m_Models->PushBack(model);
    }

    otbAppLogINFO("Models loaded");

    // Normalize input image (optional)
    StatisticsReader::Pointer  statisticsReader = StatisticsReader::New();
    MeasurementType  meanMeasurementVector;
    MeasurementType  stddevMeasurementVector;
    m_Rescalers = RescalerListType::New();

    // Classify
    m_ClassificationFilter = ClassificationFilterType::New();
    m_ClassificationFilter->SetModels(m_Models);

    // Normalize input image if asked
    bool useStatistics = IsParameterEnabled("imstat");
    if (useStatistics)
      {
      const std::vector<std::string> &statisticsFiles = GetParameterStringList("imstat");
      otbAppLogINFO("Input image normalization activated.");
      // Load input image statistics
      for (std::vector<std::string>::const_iterator statsIt = statisticsFiles.begin(), statsEnd = statisticsFiles.end(); statsIt != statsEnd; ++statsIt)
        {
        statisticsReader->SetFileName(*statsIt);
        meanMeasurementVector   = statisticsReader->GetStatisticVectorByName("mean");
        stddevMeasurementVector = statisticsReader->GetStatisticVectorByName("stddev");
        otbAppLogINFO( "mean used: " << meanMeasurementVector );
        otbAppLogINFO( "standard deviation used: " << stddevMeasurementVector );
        // Rescale vector image
        RescalerType::Pointer rescaler = RescalerType::New();
        rescaler->SetScale(stddevMeasurementVector);
        rescaler->SetShift(meanMeasurementVector);
        size_t idx = std::distance(statisticsFiles.begin(), statsIt);
        size_t imIdx = inImages->Size() == 1 ? 0 : idx;
        rescaler->SetInput(inImages->GetNthElement(imIdx));
        m_Rescalers->PushBack(rescaler);
        }
      }
    else
      {
      otbAppLogINFO("Input image normalization deactivated.");
      }

    if(IsParameterEnabled("mask"))
      {
      otbAppLogINFO("Using model mask");
      // Load mask image and cast into LabeledImageType
      MaskImageType::Pointer inMask = GetParameterUInt8Image("mask");
      std::vector<std::string> modelIds = GetParameterStringList("modelid");

      std::vector<uint8_t> modelMap(255);
      for (size_t i = 0; i < modelIds.size(); i++)
        {
        uint8_t modelId = std::stoi(modelIds[i]);
        modelMap[modelId] = i + 1;
        }

      m_ClassificationFilter->SetUseModelMask(true);
      m_ClassificationFilter->SetModelMask(inMask);
      m_ClassificationFilter->SetModelMap(modelMap);
      }


    for (size_t i = 0; i < m_Models->Size(); i++)
      {
      if (useStatistics)
        {
        m_ClassificationFilter->PushBackInput(m_Rescalers->GetNthElement(i)->GetOutput());
        }
      else
        {
        size_t imIdx = inImages->Size() == 1 ? 0 : i;
        m_ClassificationFilter->PushBackInput(inImages->GetNthElement(imIdx));
        }
      }

    if(HasValue("nodatalabel"))
      {
        m_ClassificationFilter->SetDefaultLabel(GetParameterInt("nodatalabel"));
      }

    SetParameterOutputImage<OutputImageType>("out", m_ClassificationFilter->GetOutput());
  }

  ClassificationFilterType::Pointer  m_ClassificationFilter;
  ModelListType::Pointer             m_Models;
  RescalerListType::Pointer          m_Rescalers;
};


}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::MultiModelImageClassifier)
