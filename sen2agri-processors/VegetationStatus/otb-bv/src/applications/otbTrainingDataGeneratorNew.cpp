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

#include "MetadataHelperFactory.h"
#include "CommonFunctions.h"

namespace otb
{
namespace Wrapper
{
class TrainingDataGeneratorNewApp : public Application
{

public:
  typedef TrainingDataGeneratorNewApp Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;

    typedef enum {
        MLAI_IDX=0,
        ALA_IDX = 1,
        CrownCover_IDX = 2,
        HsD_IDX = 3,
        N_IDX = 4,
        Cab_IDX = 5,
        Car_IDX = 6,
        Cdm_IDX = 7,
        CwRel_IDX = 4,
        Cbp_IDX = 9,
        Bs_IDX = 10,
        FAPAR_IDX = 11,
        FCOVER_IDX = 12
    } BV_INDEXES;


  itkNewMacro(Self)
  itkTypeMacro(TrainingDataGeneratorNewApp, otb::Application)

private:

  void DoInit()
  {
        SetName("TrainingDataGeneratorNew");
        SetDescription("Creates the training file from the simulated biophysical variable samples and from the simulated reflectances file.");

        SetDocName("TrainingDataGeneratorNew");
        SetDocLongDescription("Creates the training file from the simulated biophysical variable samples and from the simulated reflectances file.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_InputFilename, "biovarsfile", "File containing simulated biophysical variable samples");
        AddParameter(ParameterType_InputFilename, "simureflsfile", "File containing simulated reflectances file. The last 2 bands are fcover and fapar");
        AddParameter(ParameterType_Int, "bvidx", "The biophysical variable index from thesimulated biophysical variable samples file to be used.");
        SetDefaultParameterInt("bvidx", 0);
        MandatoryOff("bvidx");

        AddParameter(ParameterType_String, "redidx", "The index of the RED band in the simulated reflectances file. This is used only with addrefls and is ignored if useallrefls is set.");
        MandatoryOff("redidx");

        AddParameter(ParameterType_String, "niridx", "The index of the NIR band in the simulated reflectances file. This is used only with addrefls and is ignored if useallrefls is set.");
        MandatoryOff("niridx");

        AddParameter(ParameterType_InputFilename, "xml",
                     "Input XML file of a product that will be used to get the red and nir band indexes if redidx or niridx are not specified.");
        SetParameterDescription( "xml", "Input XML file of a product." );

        AddParameter(ParameterType_InputFilename, "laicfgs",
                     "Master file containing the LAI configuration files for each mission.");
        SetParameterDescription( "laicfgs", "Master file containing the LAI configuration files for each mission." );
        MandatoryOff("laicfgs");

        AddParameter(ParameterType_InputFilename, "laicfg",
                     "File containing the bands indexes to be used for the LAI.");
        SetParameterDescription( "laicfg", "File containing the bands indexes to be used for the LAI." );
        MandatoryOff("laicfg");

        AddParameter(ParameterType_OutputFilename, "outtrainfile", "Output training file");

        SetDocExampleParameterValue("biovarsfile", "bvfile.txt");
        SetDocExampleParameterValue("simureflsfile", "simulated_reflectances.tif");
        SetDocExampleParameterValue("bvidx", "0");
        SetDocExampleParameterValue("redidx", "B3");
        SetDocExampleParameterValue("niridx", "B8");
        SetDocExampleParameterValue("xml", "product.xml");
        SetDocExampleParameterValue("outtrainfile", "training.txt");
  }

  void DoUpdateParameters()
  {
  }
  void DoExecute()
  {
      std::string bvFileName = GetParameterString("biovarsfile");
      std::string simuReflsName = GetParameterString("simureflsfile");
      std::size_t bvIdx = GetParameterInt("bvidx");
      std::string redBandName = GetParameterString("redidx");
      std::string nirBandName = GetParameterString("niridx");

      std::string outFileName = GetParameterString("outtrainfile");

      std::string laiCfgFile;

      std::string inMetadataXml = GetParameterString("xml");
      auto factory = MetadataHelperFactory::New();
      // we are interested only in the 10m resolution as here we have the RED and NIR
      std::unique_ptr<MetadataHelper<short>> pHelper = factory->GetMetadataHelper<short>(inMetadataXml);
      // the bands are 1 based
      if(redBandName.size() == 0)
        redBandName = pHelper->GetRedBandName();
      if(nirBandName.size() == 0)
        nirBandName = pHelper->GetNirBandName();

      // Load the LAI bands configuration file from laicfgs
      if(HasValue("laicfgs")) {
          const std::string &laiCfgsFile = GetParameterString("laicfgs");
          laiCfgFile = getValueFromMissionsCfgFile(laiCfgsFile, pHelper->GetMissionName(), pHelper->GetInstrumentName());
      }

      if (laiCfgFile.length() == 0) {
          bool bHasLaiCfg = HasValue("laicfg");
          if (!bHasLaiCfg) {
              itkExceptionMacro("Either laicfg or laicfgs (if xml is present) should be configured");
          }
          laiCfgFile = GetParameterString("laicfg");
      }
      // Load the LAI bands configuration file
      const LAIBandsConfigInfos &laiCfg = loadLaiBandsConfigFile(laiCfgFile);
      // if we use NDVI or RVI we will need to have the bands for RED and NIR in the list of bands
      // for which we simulate the reflectances. The other applications will also need to handle this
      // Otherwise, the model will be created with the wrong number of bands
      std::vector<std::string> rsrColumnsFilterBandNames = laiCfg.rsrColumnFilterBandNames;
      if (laiCfg.bUseNdvi || laiCfg.bUseRvi) {
          // if RED band not present in the list, then add it
          if(std::find(rsrColumnsFilterBandNames.begin(), rsrColumnsFilterBandNames.end(), redBandName) == rsrColumnsFilterBandNames.end()) {
              rsrColumnsFilterBandNames.push_back(redBandName);
              otbAppLogINFO("Added missing RED BAND index: " << redBandName);
          }
          // if NIR band not present in the list, then add it
          if(std::find(rsrColumnsFilterBandNames.begin(), rsrColumnsFilterBandNames.end(), nirBandName) == rsrColumnsFilterBandNames.end()) {
              rsrColumnsFilterBandNames.push_back(nirBandName);
              otbAppLogINFO("Added missing NIR BAND index: " << nirBandName);
          }
          // Sort again the array
          //std::sort (rsrColumnsFilterBandNames.begin(), rsrColumnsFilterBandNames.end());
      }


      std::cout << "=================================" << std::endl;
      std::cout << "addndvi : " << laiCfg.bUseNdvi           << std::endl;
      std::cout << "addrvi : " << laiCfg.bUseRvi             << std::endl;
      std::cout << "Bands used : ";
      for (const auto &bandName: laiCfg.bandsNames) {
          std::cout << bandName << ' ';
      }
      std::cout << std::endl;
      std::cout << "=================================" << std::endl;

      printRsrBands(rsrColumnsFilterBandNames);


      try
        {
        m_SampleFile.open(bvFileName.c_str());
        }
      catch(...)
        {
        itkGenericExceptionMacro(<< "Could not open file " << bvFileName);
        }

      try
        {
        m_SimulationsFile.open(simuReflsName.c_str());
        }
      catch(...)
        {
        itkGenericExceptionMacro(<< "Could not open file " << simuReflsName);
        }


      try
        {
        m_TrainingFile.open(outFileName.c_str(), std::ofstream::out);
        }
      catch(...)
        {
        itkGenericExceptionMacro(<< "Could not open file " << outFileName);
        }

      std::string sampleLine, reflectancesLine;

      float epsilon = 0.001f;
      // read the first line as this contains the header
      std::getline(m_SampleFile, sampleLine);

      while ( std::getline(m_SampleFile, sampleLine) && std::getline(m_SimulationsFile, reflectancesLine))
      {
          // skip empty lines:
          if (sampleLine.empty() || reflectancesLine.empty())
              continue;

          std::string outline;
          std::vector<std::string> vectSamples = split(sampleLine, ' ');
          std::vector<std::string> vectRefls = split(reflectancesLine, ' ');
          if(bvIdx == FAPAR_IDX)
          {
            outline = vectRefls[vectRefls.size()-1];
          } else if (bvIdx == FCOVER_IDX) {
            outline = vectRefls[vectRefls.size()-2];
          } else {
              if(bvIdx < vectSamples.size())
              {
                outline = vectSamples[bvIdx];
              } else {
                itkGenericExceptionMacro(<< "Index " << bvIdx << " is not suitable for samples line " << sampleLine);
              }
          }

          // remove the last 2 values from the array (fAPAR and fCover)
          vectRefls.pop_back();
          vectRefls.pop_back();

          size_t nbFileReflBands = vectRefls.size();
          const std::vector<int> &bandsIndexes = CheckAndTranslateBandIndexes(pHelper, rsrColumnsFilterBandNames, vectRefls, laiCfg);

          // we use all the bands for the sensor
          for(size_t i = 0; i<nbFileReflBands; i++) {
              // check if the current band is configured
              if(std::find(bandsIndexes.begin(), bandsIndexes.end(), i) != bandsIndexes.end()) {
                  outline += " ";
                  outline += vectRefls[i];
              }
          }

          if (laiCfg.bUseNdvi || laiCfg.bUseRvi) {
              // add NDVI and RVI values
              if(redBandName.size() > 0 && nirBandName.size() > 0) {
                  int relRedBandIdx = getBandIndexInRsrVect(rsrColumnsFilterBandNames, redBandName);
                  int relNirBandIdx = getBandIndexInRsrVect(rsrColumnsFilterBandNames, nirBandName);
                  if(relRedBandIdx >=0 && relNirBandIdx >= 0) {
                      float fRedVal = std::stof(vectRefls[relRedBandIdx]);
                      float fNirVal = std::stof(vectRefls[relNirBandIdx]);
                      float ndvi = (fNirVal-fRedVal)/(fNirVal+fRedVal+epsilon);
                      float  rvi = fNirVal/(fRedVal+epsilon);
                      // normalize value for RVI
                      if(rvi < 0.000001 || std::isnan(rvi)) {
                          rvi = 0;
                      } else {
                          if(rvi > 30 || std::isinf(rvi)) {
                              rvi = 30;
                          }
                      }
                      std::ostringstream ss;
                      if(laiCfg.bUseNdvi) {
                        ss << " " << ndvi;
                      }
                      if(laiCfg.bUseRvi) {
                        ss << " " << rvi;
                      }
                      outline += ss.str();
                  } else {
                      itkGenericExceptionMacro(<< "RED or NIR relative bands indexes cannot be determined!");
                  }
              } else {
                  itkGenericExceptionMacro(<< "Requested use of RVI or NDVI but RED or NIR band is not available!");
              }
          }
          m_TrainingFile << outline << std::endl;
      }
  }

  std::vector<int> CheckAndTranslateBandIndexes(const std::unique_ptr<MetadataHelper<short>> &pHelper,
                                                const std::vector<std::string> &rsrColumnsFilterBandNames,
                                                const std::vector<std::string> &vectRefls,
                                                const LAIBandsConfigInfos &laiCfg) {
      size_t nbFileReflBands = vectRefls.size();
      std::vector<int> bandsIndexes;
      // check if a filter on the RSR bands was made or we have all RSR bands
      if (rsrColumnsFilterBandNames.size() > 0) {
          // validations of the RSR bands
          if (nbFileReflBands != rsrColumnsFilterBandNames.size()) {
              itkGenericExceptionMacro(<< "Invalid bands number. Different from the number defined in RSR_COLS_FILTER ("
                                       << nbFileReflBands << " != " << rsrColumnsFilterBandNames.size() << ")");
          }
          // Translate the bands indexes to the correct values
          // first check that the LAI bands indexes are within the rsr columns indexes
          for (size_t i = 0; i<laiCfg.bandsNames.size(); i++) {
              int bandIdx = getBandIndexInRsrVect(rsrColumnsFilterBandNames, laiCfg.bandsNames[i]);
              if (bandIdx == -1) {
                  itkGenericExceptionMacro(<< "Incorrect config in LAI config. Band index " << laiCfg.bandsNames[i] << " is not in the defined RSR indexes");
              } else {
                  // add the found index in the vector as the translated index of the band but 1 based
                  bandsIndexes.push_back(bandIdx);
              }
          }

      } else {
          // check that the indexes are smaller than the number of bands
          const std::vector<std::string> &allBands = pHelper->GetAllBandNames();
          for (const std::string &bandName: laiCfg.bandsNames) {
              std::ptrdiff_t pos = std::find(allBands.begin(), allBands.end(), bandName) -
                      allBands.begin();
              if(pos >= (std::ptrdiff_t)allBands.size()) {
                  itkGenericExceptionMacro(<< "Band " << bandName << " cannot be found in product band ");
              }
              if(pos > nbFileReflBands) {
                  itkGenericExceptionMacro(<< "Number of LAI band names higher than the number of valid reflectance bands " << nbFileReflBands);
              }
              bandsIndexes.push_back(pos);
          }
      }

      return bandsIndexes;
  }

  int getBandIndexInRsrVect(const std::vector<std::string> &rsrColumnsFilterNames, std::string bandName) {
      std::ptrdiff_t pos = std::find(rsrColumnsFilterNames.begin(), rsrColumnsFilterNames.end(), bandName) -
              rsrColumnsFilterNames.begin();
      if(pos >= (std::ptrdiff_t)rsrColumnsFilterNames.size()) {
          return -1;
      }
      // return the found index
      return pos;
  }

  void printRsrBands(const std::vector<std::string> &rsrColumnsFilterIndexes) {
      std::stringstream ss;
      ss << "RSR bands indexes used:" << std::endl;
      for(size_t i = 0; i< rsrColumnsFilterIndexes.size(); ++i) {
        ss << i<< " " << rsrColumnsFilterIndexes[i] << std::endl;
      }
      ss << std::endl;

      otbAppLogINFO(""<<ss.str());
  }


  // the input file
  std::ifstream m_SampleFile;
  // the input file
  std::ifstream m_SimulationsFile;
  // the output file
  std::ofstream m_TrainingFile;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::TrainingDataGeneratorNewApp)


