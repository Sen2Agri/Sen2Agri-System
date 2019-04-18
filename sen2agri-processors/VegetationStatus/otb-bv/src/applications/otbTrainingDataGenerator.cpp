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

namespace otb
{
namespace Wrapper
{
class TrainingDataGeneratorApp : public Application
{

public:
  typedef TrainingDataGeneratorApp Self;
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
  itkTypeMacro(TrainingDataGeneratorApp, otb::Application)

private:

  void DoInit()
  {
        SetName("TrainingDataGenerator");
        SetDescription("Creates the training file from the simulated biophysical variable samples and from the simulated reflectances file.");

        SetDocName("TrainingDataGenerator");
        SetDocLongDescription("Creates the training file from the simulated biophysical variable samples and from the simulated reflectances file.");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_InputFilename, "biovarsfile", "File containing simulated biophysical variable samples");
        AddParameter(ParameterType_InputFilename, "simureflsfile", "File containing simulated reflectances file. The last 2 bands are fcover and fapar");
        AddParameter(ParameterType_Int, "bvidx", "The biophysical variable index from thesimulated biophysical variable samples file to be used.");
        SetDefaultParameterInt("bvidx", 0);
        MandatoryOff("bvidx");

        AddParameter(ParameterType_Int, "redidx", "The index of the RED band in the simulated reflectances file. This is used only with addrefls and is ignored if useallrefls is set.");
        SetDefaultParameterInt("redidx", -1);
        MandatoryOff("redidx");

        AddParameter(ParameterType_Int, "blueidx", "The index of the BLUE band in the simulated reflectances file. This is used only with addrefls and is ignored if useallrefls is set.");
        SetDefaultParameterInt("blueidx", -1);
        MandatoryOff("blueidx");

        AddParameter(ParameterType_Int, "niridx", "The index of the NIR band in the simulated reflectances file. This is used only with addrefls and is ignored if useallrefls is set.");
        SetDefaultParameterInt("niridx", -1);
        MandatoryOff("niridx");

        AddParameter(ParameterType_Int, "addrefls", "Specifies if the simulated reflectance values bands should be also added in the training file or only NDVI and RVI.");
        SetDefaultParameterInt("addrefls", 1);
        MandatoryOff("addrefls");

        AddParameter(ParameterType_Int, "useallrefls", "Boolean specifying if all reflectances should be used. If this is set to 1, then the RED and NIR indexes are not needed anymore.");
        SetDefaultParameterInt("useallrefls", 1);
        MandatoryOff("useallrefls");

        AddParameter(ParameterType_Int, "usendvi", "Add NDVI band for the training data.");
        MandatoryOff("usendvi");
        SetDefaultParameterInt("usendvi", 0);

        AddParameter(ParameterType_Int, "uservi", "Add RVI band for the training data.");
        MandatoryOff("uservi");
        SetDefaultParameterInt("uservi", 0);

        AddParameter(ParameterType_Int, "useblue", "Use blue band for the training data.");
        MandatoryOff("useblue");
        SetDefaultParameterInt("useblue", 0);

        AddParameter(ParameterType_InputFilename, "xml",
                     "Input XML file of a product that will be used to get the red and nir band indexes if redidx or niridx are not specified.");
        SetParameterDescription( "xml", "Input XML file of a product." );
        MandatoryOff("xml");


        AddParameter(ParameterType_OutputFilename, "outtrainfile", "Output training file");


        SetDocExampleParameterValue("biovarsfile", "bvfile.txt");
        SetDocExampleParameterValue("simureflsfile", "simulated_reflectances.tif");
        SetDocExampleParameterValue("bvidx", "0");
        SetDocExampleParameterValue("redidx", "0");
        SetDocExampleParameterValue("niridx", "2");
        SetDocExampleParameterValue("addrefls", "1");
        SetDocExampleParameterValue("useallrefls", "1");
        SetDocExampleParameterValue("xml", "product.xml");
        SetDocExampleParameterValue("outtrainfile", "training.txt");
  }

  void DoUpdateParameters()
  {
  }
  void DoExecute()
  {
      bool bUseRvi = false;
      bool bUseNdvi = false;
      bool bUseBlueBand = false;

      std::string bvFileName = GetParameterString("biovarsfile");
      std::string simuReflsName = GetParameterString("simureflsfile");
      std::size_t bvIdx = GetParameterInt("bvidx");
      int redIdx = GetParameterInt("redidx");
      int blueIdx = GetParameterInt("blueidx");
      int nirIdx = GetParameterInt("niridx");
      bool addRefls = (GetParameterInt("addrefls") != 0);
      if(!addRefls && (redIdx < 0 && nirIdx < 0)) {
          itkGenericExceptionMacro(<< "You should either set addrefls to 1 OR set the redidx and niridx to valid values");
      }

      if(HasValue("usendvi")) {
          bUseNdvi = (GetParameterInt("usendvi") != 0);
      }
      if(HasValue("uservi")) {
          bUseRvi = (GetParameterInt("uservi") != 0);
      }
      if(HasValue("useblue")) {
          bUseBlueBand = (GetParameterInt("useblue") != 0);
      }
      std::cout << "=================================" << std::endl;
      std::cout << "addndvi : " << bUseNdvi           << std::endl;
      std::cout << "addrvi : " << bUseRvi             << std::endl;
      std::cout << "addblue : " << bUseBlueBand       << std::endl;
      std::cout << "=================================" << std::endl;

      std::string outFileName = GetParameterString("outtrainfile");
      bool bUseAllReflectanceBands = (GetParameterInt("useallrefls") != 0);

      int nBandsNo = -1;
      if(redIdx == -1 || nirIdx == -1) {
          if(HasValue("xml")) {
              std::string inMetadataXml = GetParameterString("xml");
              auto factory = MetadataHelperFactory::New();
              // we are interested only in the 10m resolution as here we have the RED and NIR
              std::unique_ptr<MetadataHelper<short>> pHelper = factory->GetMetadataHelper<short>(inMetadataXml);
              const std::vector<std::string> &bandNames = pHelper->GetBandNamesForResolution(pHelper->GetProductResolutions()[0]);
              nBandsNo = bandNames.size();
              // the bands are 1 based
              if(redIdx == -1)
                redIdx = std::distance(bandNames.begin(), find(bandNames.begin(), bandNames.end(), pHelper->GetRedBandName()));
              if(blueIdx == -1)
                blueIdx = std::distance(bandNames.begin(), find(bandNames.begin(), bandNames.end(), pHelper->GetBlueBandName()));
              if(nirIdx == -1)
                nirIdx = std::distance(bandNames.begin(), find(bandNames.begin(), bandNames.end(), pHelper->GetNirBandName()));
          }
      }

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

          if(addRefls) {
              // remove the last 2 values from the array (fAPAR and fCover)
              vectRefls.pop_back();
              vectRefls.pop_back();

              if(bUseAllReflectanceBands) {
                  // we use the bands number extracted from metadata or all the bands loaded from simulation file
                  if(nBandsNo <= 0)
                      nBandsNo = vectRefls.size();
                  for(int i = 0; i<nBandsNo; i++) {
                      if (bUseBlueBand || (i != blueIdx)) {
                          outline += " ";
                          outline += vectRefls[i];
                      }
                  }
              } else {
                  // use only RED and NIR
                  if(redIdx < (int)vectRefls.size() && nirIdx < (int)vectRefls.size()) {
                    outline += " ";
                    outline += vectRefls[redIdx];
                    outline += " ";
                    outline += vectRefls[nirIdx];
                  }
              }
              //outline += accumulate( vectRefls.begin(), vectRefls.end(), std::string(" ") );
          }
          // TODO trim line
          // TODO: The angles are not added
          /*
          if add_angles:
              angles = `simuPars['solarZenithAngle']`+" "+`simuPars['sensorZenithAngle']`+" "+`simuPars['solarSensorAzimuth']`
              outline += " "+angles
          */

          // add NDVI and RVI values
          if(redIdx >= 0 && nirIdx >= 0) {
              if(redIdx < (int)vectRefls.size() && nirIdx < (int)vectRefls.size()) {
                  float fRedVal = std::stof(vectRefls[redIdx]);
                  float fNirVal = std::stof(vectRefls[nirIdx]);
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
                  if(bUseNdvi) {
                    ss << " " << ndvi;
                  }
                  if(bUseRvi) {
                    ss << " " << rvi;
                  }
                  outline += ss.str();
              }
          }
          m_TrainingFile << outline << std::endl;
      }
  }

  std::vector<std::string> split(std::string str, char delimiter) {
    std::vector<std::string> internal;
    std::stringstream ss(str); // Turn the string into a stream.
    std::string tok;

    while(std::getline(ss, tok, delimiter)) {
      internal.push_back(tok);
    }

    return internal;
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

OTB_APPLICATION_EXPORT(otb::Wrapper::TrainingDataGeneratorApp)


