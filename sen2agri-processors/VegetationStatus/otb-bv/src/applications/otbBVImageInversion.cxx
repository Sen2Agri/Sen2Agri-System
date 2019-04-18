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
#include "otbWrapperChoiceParameter.h"

#include <fstream>
#include <string>
#include <memory>

#include "otbBVUtil.h"

#include "otbMachineLearningModelFactory.h"
#include "otbNeuralNetworkRegressionMachineLearningModel.h"
#include "otbSVMMachineLearningModel.h"
#include "otbRandomForestsMachineLearningModel.h"
#include "otbMultiLinearRegressionModel.h"
#include "itkListSample.h"
#include "GlobalDefs.h"
#include "MetadataHelperFactory.h"
#include "dirent.h"
#include "itkBinaryFunctorImageFilter.h"

typedef double PrecisionType;
typedef itk::FixedArray<PrecisionType, 1> OutputSampleType;
typedef itk::VariableLengthVector<PrecisionType> InputSampleType;
typedef itk::Statistics::ListSample<OutputSampleType> ListOutputSampleType;
typedef itk::Statistics::ListSample<InputSampleType> ListInputSampleType;
typedef otb::MachineLearningModel<PrecisionType, PrecisionType> ModelType;
typedef ModelType::Pointer ModelPointerType;
typedef otb::NeuralNetworkRegressionMachineLearningModel<PrecisionType, 
                                                         PrecisionType> 
NeuralNetworkType;
typedef otb::RandomForestsMachineLearningModel<PrecisionType, 
                                               PrecisionType> RFRType;
typedef otb::SVMMachineLearningModel<PrecisionType, PrecisionType> SVRType;
typedef otb::MultiLinearRegressionModel<PrecisionType> MLRType;

namespace otb
{

/** Binary functor image filter which produces a vector image with a
* number of bands different from the input images */
template <class TInputImage1, class TInputImage2, class TOutputImage,
          class TFunctor>
class ITK_EXPORT BinaryFunctorImageFilterWithNBands :
    public itk::BinaryFunctorImageFilter< TInputImage1, TInputImage2, TOutputImage, TFunctor >
{
public:
  typedef BinaryFunctorImageFilterWithNBands Self;
  typedef itk::BinaryFunctorImageFilter< TInputImage1, TInputImage2, TOutputImage,
                                        TFunctor > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Macro defining the type*/
  itkTypeMacro(BinaryFunctorImageFilterWithNBands, SuperClass);

  /** Accessors for the number of bands*/
  itkSetMacro(NumberOfOutputBands, unsigned int);
  itkGetConstMacro(NumberOfOutputBands, unsigned int);

protected:
  BinaryFunctorImageFilterWithNBands() {}
  virtual ~BinaryFunctorImageFilterWithNBands() {}

  void GenerateOutputInformation()
  {
    Superclass::GenerateOutputInformation();
    this->GetOutput()->SetNumberOfComponentsPerPixel( m_NumberOfOutputBands );
  }
private:
  BinaryFunctorImageFilterWithNBands(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  unsigned int m_NumberOfOutputBands;


};

/** Unary functor image filter which produces a vector image with a
* number of bands different from the input images */
template <class TInputImage, class TOutputImage, 
          class TFunctor>
class ITK_EXPORT UnaryFunctorImageFilterWithNBands : 
    public itk::UnaryFunctorImageFilter< TInputImage, TOutputImage, TFunctor >
{
public:
  typedef UnaryFunctorImageFilterWithNBands Self;
  typedef itk::UnaryFunctorImageFilter< TInputImage, TOutputImage, 
                                        TFunctor > Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Macro defining the type*/
  itkTypeMacro(UnaryFunctorImageFilterWithNBands, SuperClass);

  /** Accessors for the number of bands*/
  itkSetMacro(NumberOfOutputBands, unsigned int);
  itkGetConstMacro(NumberOfOutputBands, unsigned int);
  
protected:
  UnaryFunctorImageFilterWithNBands() {}
  virtual ~UnaryFunctorImageFilterWithNBands() {}

  void GenerateOutputInformation()
  {
    Superclass::GenerateOutputInformation();
    this->GetOutput()->SetNumberOfComponentsPerPixel( m_NumberOfOutputBands );
  }
private:
  UnaryFunctorImageFilterWithNBands(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  unsigned int m_NumberOfOutputBands;


};
template <typename InputPixelType, typename OutputPixelType>
class BVEstimationFunctor
{
public:
  BVEstimationFunctor() = default;
  BVEstimationFunctor(ModelType* model,
                      const NormalizationVectorType& normalization) : 
    m_Model{model}, m_Normalization{normalization} {}

  ~BVEstimationFunctor() {};
  
  inline
  OutputPixelType operator ()(const InputPixelType& in_pix)
  {
    bool normalization{m_Normalization!=NormalizationVectorType{}};
    OutputPixelType pix{};
    pix.SetSize(1);
    auto nbInputVariables = in_pix.GetSize();
    InputSampleType inputValue;
    inputValue.Reserve(nbInputVariables);
    int cnt = 0;
    for(size_t var = 0; var < nbInputVariables; ++var)
      {
        if(!IsNoDataValue(in_pix[var])) {
            inputValue[cnt] = in_pix[var];
            if( normalization )
                inputValue[cnt] = normalize(inputValue[cnt], m_Normalization[var]);
            cnt++;
        }
    }
    if(cnt < nbInputVariables) {
        pix[0] = NO_DATA_VALUE;
    } else {
        OutputSampleType outputValue = m_Model->Predict(inputValue);
        pix[0] = outputValue[0];
        if( normalization )
          pix[0] = denormalize(outputValue[0],
                               m_Normalization[nbInputVariables]);
    }
    return pix;
  }

  inline
  OutputPixelType operator ()(const InputPixelType& in_pix, const InputPixelType& maskPix)
  {
      if(maskPix[0] != IMG_FLG_LAND) {
          OutputPixelType pix{};
          pix.SetSize(1);
          pix[0] = NO_DATA_VALUE;
          return pix;
      }
      return (*this)(in_pix);
  }

  bool operator !=(const BVEstimationFunctor& other) const
  {
    return (this->m_Model!=other.m_Model ||
            this->m_Normalization!=other.m_Normalization);
  }

  bool operator ==(const BVEstimationFunctor& other) const
  {
    return !(*this!=other);
  }

  bool IsNoDataValue(float fValue)
  {
      return (fValue < 0);
      //return fabs(fValue - NO_DATA_VALUE) < NO_DATA_EPSILON;
  }


protected:
  ModelPointerType m_Model;
  NormalizationVectorType m_Normalization;

};

namespace Wrapper
{

class BVImageInversion : public Application
{
public:
/** Standard class typedefs. */
  typedef BVImageInversion     Self;
  typedef Application                   Superclass;
  typedef itk::SmartPointer<Self>       Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  
/** Standard macro */
  itkNewMacro(Self);
  itkTypeMacro(BVImageInversion, otb::Application);

  using FunctorType = BVEstimationFunctor<FloatVectorImageType::PixelType, 
                                          FloatVectorImageType::PixelType>;
  using FilterType = UnaryFunctorImageFilterWithNBands<FloatVectorImageType,
                                                       FloatVectorImageType,
                                                       FunctorType>;
  using MaskedFilterType = BinaryFunctorImageFilterWithNBands<FloatVectorImageType,
                                                       FloatVectorImageType,
                                                       FloatVectorImageType,
                                                       FunctorType>;

private:
  void DoInit()
  {
    SetName("BVImageInversion");
    SetDescription("Estimate biophysical variables for every pixel of an image using an inversion of Prospect+Sail.");

    AddParameter(ParameterType_InputImage, "in", "Input Image");
    SetParameterDescription("in","Input image.");

    AddParameter(ParameterType_InputImage, "msks", "Masks flags used for masking final LAI values");
    MandatoryOff("msks");

    AddParameter(ParameterType_InputFilename, "model", "File containing the regression model.");
    SetParameterDescription( "model", "File containing the regression model.");
    MandatoryOff("model");

    AddParameter(ParameterType_InputFilename, "modelfile", "File containing paths to the regression model.");
    SetParameterDescription( "modelfile", "File containing paths to the regression model.");
    MandatoryOff("modelfile");

    AddParameter(ParameterType_InputFilename, "xml",
                 "Input XML file of a product containing angles. If specified, the angles above will be ignored.");
    SetParameterDescription( "xml", "Input XML file of a product containing angles." );
    MandatoryOff("xml");

    AddParameter(ParameterType_String, "modelsfolder", "Folder containing the regression models.");
    SetParameterDescription( "modelsfolder", "Folder containing the regression models.");
    MandatoryOff("modelsfolder");

    AddParameter(ParameterType_String, "modelprefix", "Prefix of the desired model found in the specified folder.");
    SetParameterDescription( "modelprefix", "Prefix of the desired model found in the specified folder.");
    MandatoryOff("modelprefix");

    AddParameter(ParameterType_OutputImage, "out", "Output Image");
    SetParameterDescription("out","Output image.");

    AddRAMParameter();

    AddParameter(ParameterType_InputFilename, "normalization", "Input file containing min and max values per sample component.");
    SetParameterDescription( "normalization", "Input file containing min and max values per sample component. This file can be produced by the invers model learning application. If no file is given as parameter, the variables are not normalized." );
    MandatoryOff("normalization");

  }

  virtual ~BVImageInversion()
  {
  }


  void DoUpdateParameters()
  {
    // Nothing to do here : all parameters are independent
  }

  void DoExecute()
  {
        std::string modelFileName;
        if(!HasValue("model") && !HasValue("modelfile")) {
            if(HasValue("modelsfolder") && HasValue("modelprefix") && HasValue("xml")) {
                std::vector<std::string> modelFiles;
                std::string modelsFolder = GetParameterString("modelsfolder");
                std::string modelsPrefix = GetParameterString("modelprefix");
                getModelsListFromFolder(modelsFolder, modelsPrefix, modelFiles);
                if(modelFiles.size() > 0) {
                    std::string inMetadataXml = GetParameterString("xml");
                    if (inMetadataXml.empty())
                    {
                        itkExceptionMacro("No input metadata XML set...; please set the input XML");
                    }
                    auto factory = MetadataHelperFactory::New();
                    // we are interested only in the 10m resolution as here we have the RED and NIR
                    std::unique_ptr<MetadataHelper<short>> pHelper = factory->GetMetadataHelper<short>(inMetadataXml);
                    std::string foundModelName = getModelFileName(modelFiles, pHelper);
                    if(foundModelName == "") {
                        itkGenericExceptionMacro(<< "No suitable model name with the prefix " << modelsPrefix <<
                                                 " was found for the product " << inMetadataXml
                                                 << " in the folder " << modelsFolder);
                    }
                    modelFileName = foundModelName;
                }
            }
            if(modelFileName.size() == 0)
                itkExceptionMacro("You should specify at least (model or the modelslist file name) or "
                                  "(modelsfolder & modelprefix & xml)");
        } else {
            if(HasValue("model")) {
                modelFileName = GetParameterString("model");
            }
            // if we have also a file as parameter, get the model path from file
            if(HasValue("modelfile")) {
                std::vector<std::string> modelsFileNames;
                readFileLines(GetParameterString("modelfile"), modelsFileNames);
                if(modelsFileNames.size() > 0)
                    modelFileName = modelsFileNames[0];
            }
        }

        otbAppLogINFO("Using model file: "<< modelFileName <<std::endl);

        // read output info of the input image
        FloatVectorImageType::Pointer input_image = this->GetParameterImage("in");
        auto nb_bands = input_image->GetNumberOfComponentsPerPixel();
        otbAppLogINFO("Input image has " << nb_bands << " bands."<< std::endl);
        auto nbInputVariables = nb_bands;

        NormalizationVectorType var_minmax{};
        if( HasValue( "normalization" )==true )
        {
            otbAppLogINFO("Variable normalization."<< std::endl);
            var_minmax = read_normalization_file(GetParameterString("normalization"));
            if(var_minmax.size()!=nbInputVariables+1)
                itkGenericExceptionMacro(<< "Normalization file ("<< var_minmax.size()
                                 << " - 1) is not coherent with the number of "
                                 << "input variables ("<< nbInputVariables 
                                 <<").");
            for(size_t var = 0; var < nbInputVariables; ++var)
                otbAppLogINFO("Variable "<< var+1 << " min=" << var_minmax[var].first <<
                      " max=" << var_minmax[var].second <<std::endl);
            otbAppLogINFO("Output min=" << var_minmax[nbInputVariables].first <<
                    " max=" << var_minmax[nbInputVariables].second 
                    << std::endl)
        }

        ModelType* regressor;
        auto nn_regressor = NeuralNetworkType::New();
        auto svr_regressor = SVRType::New();
        auto rfr_regressor = RFRType::New();
        auto mlr_regressor = MLRType::New();
        if(nn_regressor->CanReadFile(modelFileName))
          {
          regressor = dynamic_cast<ModelType*>(nn_regressor.GetPointer());
          otbAppLogINFO("Applying NN regression ..." << std::endl);
          }
        else if(svr_regressor->CanReadFile(modelFileName))
          {
          regressor = dynamic_cast<ModelType*>(svr_regressor.GetPointer());
          otbAppLogINFO("Applying SVR regression ..." << std::endl);
          }
        else if(rfr_regressor->CanReadFile(modelFileName))
          {
          regressor = dynamic_cast<ModelType*>(rfr_regressor.GetPointer());
          otbAppLogINFO("Applying RF regression ..." << std::endl);
          }
        else if(mlr_regressor->CanReadFile(modelFileName))
          {
          regressor = dynamic_cast<ModelType*>(mlr_regressor.GetPointer());
          otbAppLogINFO("Applying MLR regression ..." << std::endl);
          }
        else
          {
          itkGenericExceptionMacro(<< "Model in file " << modelFileName
                                   << " is not valid.\n");
          }
        regressor->Load(modelFileName);

        bool bHasMsks = HasValue("msks");
        if(bHasMsks) {
            m_msksImg = GetParameterFloatVectorImage("msks");
            bv_MaskedFilter = MaskedFilterType::New();
            bv_MaskedFilter->SetFunctor(FunctorType(regressor,var_minmax));
            bv_MaskedFilter->SetInput1(input_image);
            bv_MaskedFilter->SetInput2(m_msksImg);
            bv_MaskedFilter->SetNumberOfOutputBands(1);
            SetParameterOutputImage("out", bv_MaskedFilter->GetOutput());
        } else {

            //instantiate a functor with the regressor and pass it to the
            //unary functor image filter pass also the normalization values
            bv_filter = FilterType::New();
            bv_filter->SetFunctor(FunctorType(regressor,var_minmax));
            bv_filter->SetInput(input_image);
            bv_filter->SetNumberOfOutputBands(1);
            SetParameterOutputImage("out", bv_filter->GetOutput());
        }
  }

  void readFileLines(const std::string &fileName, std::vector<std::string> &outLines) {
      std::ifstream isFile;
      isFile.open(fileName);
      if (!isFile.is_open()) {
          itkExceptionMacro("Can't open file containing model path for reading!");
      }

      // read the file lines
      std::string value;
      while (std::getline(isFile, value)) {
          outLines.push_back(value);
      }

      // close the file
      isFile.close();

  }

  void getModelsListFromFolder(const std::string &folderName, const std::string &modelPrefix, std::vector<std::string> &outModelsList) {
      DIR *dir;
      struct dirent *ent;
      if ((dir = opendir (folderName.c_str())) != NULL) {
        /* print all the files and directories within directory */
        while ((ent = readdir (dir)) != NULL) {
            std::string fileName = ent->d_name;
            if(fileName.find(modelPrefix) == 0) {
                outModelsList.push_back(folderName + "/" + fileName);
            }
        }
        closedir (dir);
      } else {
          itkGenericExceptionMacro(<< "Cannot open folder " << folderName << "\n");
      }
  }

  std::string getModelFileName(const std::vector<std::string> &modelsList, const std::unique_ptr<MetadataHelper<short>> &pHelper) {
      double fSolarZenith;
      double fSensorZenith;
      double fRelAzimuth;

      MeanAngles_Type solarAngles = pHelper->GetSolarMeanAngles();
      double relativeAzimuth = pHelper->GetRelativeAzimuthAngle();

      for(int i = 0; i<(int)modelsList.size(); i++) {
          std::string modelName = modelsList[i];

          if(parseModelFileName(modelName, fSolarZenith, fSensorZenith, fRelAzimuth)) {
              if(pHelper->HasBandMeanAngles()) {
                  int nTotalBandsNo = pHelper->GetBandsPresentInPrdTotalNo();
                  for(int j = 0; j<nTotalBandsNo; j++) {
                      MeanAngles_Type sensorBandAngles = pHelper->GetSensorMeanAngles(j);
                      if(inRange(fSolarZenith, 0.5, solarAngles.zenith) &&
                         inRange(fSensorZenith, 0.5, sensorBandAngles.zenith) &&
                         inRange(fRelAzimuth, 0.5, relativeAzimuth))
                      {
                          return modelName;
                      }
                  }
              } else if(pHelper->HasGlobalMeanAngles()) {
                MeanAngles_Type sensorBandAngles = pHelper->GetSensorMeanAngles();
                if(inRange(fSolarZenith, 0.5, solarAngles.zenith) &&
                   inRange(fSensorZenith, 0.5, sensorBandAngles.zenith) &&
                   inRange(fRelAzimuth, 0.5, relativeAzimuth))
                {
                    return modelName;
                }
              } else {
                  otbAppLogWARNING("There are no angles for this mission? " << pHelper->GetMissionName());
              }
          } else {
              otbAppLogWARNING("Invalid model name found in list: " << modelName);
          }
      }

      otbAppLogWARNING("NO APPROPRIATE MODEL NAME FOUND!");
      if(modelsList.size() > 0) {
        otbAppLogWARNING("Using the first model in the list: " << modelsList[0]);
        return modelsList[0];
      }
      return "";
  }

  bool parseModelFileName(const std::string &modelFileName, double &solarZenith, double &sensorZenith, double &relAzimuth) {
      //The expected file name format is:
      // [FILEPREFIX_]THETA_S_<solarzenith>_THETA_V_<sensorzenith>_REL_PHI_%f
      std::size_t nThetaSPos = modelFileName.find("_THETA_S_");
      if (nThetaSPos == std::string::npos)
        return false;

      std::size_t nThetaVPos = modelFileName.find("_THETA_V_");
      if (nThetaVPos == std::string::npos)
        return false;

      std::size_t nRelPhiPos = modelFileName.find("_REL_PHI_");
      if (nRelPhiPos == std::string::npos)
        return false;

      std::size_t nExtPos = modelFileName.find(".txt");
      if (nExtPos == std::string::npos)
        return false;

      int nThetaSStartIdx = nThetaSPos + strlen("_THETA_S_");
      std::string strThetaS = modelFileName.substr(nThetaSStartIdx, nThetaVPos - nThetaSStartIdx);

      int nThetaVStartIdx = nThetaVPos + strlen("_THETA_V_");
      std::string strThetaV = modelFileName.substr(nThetaVStartIdx, nRelPhiPos - nThetaVStartIdx);

      int nRelPhiStartIdx = nRelPhiPos + strlen("_REL_PHI_");
      std::string strRelPhi = modelFileName.substr(nRelPhiStartIdx, nExtPos - nRelPhiStartIdx);

      solarZenith = ::atof(strThetaS.c_str());
      sensorZenith = ::atof(strThetaV.c_str());
      relAzimuth = ::atof(strRelPhi.c_str());

      return true;
  }

  bool inRange(double middle, double distance, double value) {
        if((value >= (middle - distance)) &&
           (value <= (middle + distance))) {
              return true;
        }

        return false;
  }


  FilterType::Pointer bv_filter;
  MaskedFilterType::Pointer bv_MaskedFilter;
  FloatVectorImageType::Pointer m_msksImg;
};

}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::BVImageInversion)
