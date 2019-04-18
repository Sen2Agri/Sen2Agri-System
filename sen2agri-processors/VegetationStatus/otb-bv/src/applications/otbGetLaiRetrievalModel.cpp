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
 
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbVectorImage.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbImageListToVectorImageFilter.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbBandMathImageFilter.h"
#include "MetadataHelperFactory.h"


namespace otb
{
namespace Wrapper
{
class GetLaiRetrievalModel : public Application
{
public:
  typedef GetLaiRetrievalModel Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

  itkNewMacro(Self)
  itkTypeMacro(GetLaiRetrievalModel, otb::Application)

private:

  void DoInit()
  {
        SetName("GetLaiRetrievalModel");
        SetDescription("Selects a model for LAI retrieval based on the product angles.");

        SetDocName("GetLaiRetrievalModel");
        SetDocLongDescription("Selects a model for LAI retrieval based on the product angles.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_InputFilenameList, "ilmodels", "The list containing the models files.");
        AddParameter(ParameterType_InputFilenameList, "ilerrmodels", "The list containing the error estimation models files.");
        AddParameter(ParameterType_String, "xml", "Product Metadata XML File");
        AddParameter(ParameterType_OutputFilename, "outm", "The selected model.");
        AddParameter(ParameterType_OutputFilename, "outerr", "The selected error estimation model.");

        SetDocExampleParameterValue("ilmodels", "model1.txt model2.txt");
        SetDocExampleParameterValue("ilerrmodels", "err_est_model1.txt err_est_model2.txt");
        SetDocExampleParameterValue("xml", "product_metadata.xml");
        SetDocExampleParameterValue("outm", "selected_model.txt");
        SetDocExampleParameterValue("outerr", "selected_error_estimation_model.txt");
  }

  void DoUpdateParameters()
  {
  }
  void DoExecute()
  {
    std::vector<std::string> modelsList = this->GetParameterStringList("ilmodels");
    std::vector<std::string> errEstimationModelsList = this->GetParameterStringList("ilerrmodels");
    std::string inMetadataXml = GetParameterString("xml");
    if (inMetadataXml.empty())
    {
        itkExceptionMacro("No input metadata XML set...; please set the input image");
    }
    auto factory = MetadataHelperFactory::New();
    // we are interested only in the 10m resolution as here we have the RED and NIR
    std::unique_ptr<MetadataHelper<short>> pHelper = factory->GetMetadataHelper<short>(inMetadataXml);

    std::string foundModelName = getModelFileName(modelsList, pHelper);
    std::string foundErrModelName = getModelFileName(errEstimationModelsList, pHelper);

    if(foundModelName == "") {
        itkGenericExceptionMacro(<< "No suitable model name was found for the product " << inMetadataXml);
    }
    if(foundErrModelName == "") {
        itkGenericExceptionMacro(<< "No suitable error estimation model name was found for the product " << inMetadataXml);
    }

    std::string outFileName = GetParameterString("outm");
    std::string outErrFileName = GetParameterString("outerr");

    writeModelNameToOutput(outFileName, foundModelName);
    writeModelNameToOutput(outErrFileName, foundErrModelName);
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

  void writeModelNameToOutput(const std::string &outFileName, const std::string &modelName) {
      std::ofstream  modelFileContainerFile;

      modelFileContainerFile.open(outFileName);
      if (!modelFileContainerFile.is_open()) {
          itkExceptionMacro("Can't open file for writing the output!");
      }

      otbAppLogINFO("" << "================================================" << std::endl);
      otbAppLogINFO("" << "Extracted model name " << modelName << std::endl);
      otbAppLogINFO("" << "================================================" << std::endl);

      modelFileContainerFile << modelName;

      // close the file containing the model name
      modelFileContainerFile.close();
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
};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::GetLaiRetrievalModel)


