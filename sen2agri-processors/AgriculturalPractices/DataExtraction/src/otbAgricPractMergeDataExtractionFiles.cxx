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

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "tinyxml_utils.hpp"
#include "string_utils.hpp"
#include "CommonFunctions.h"


namespace otb
{
namespace Wrapper
{
class AgricPractMergeDataExtractionFiles : public Application
{
public:
    /** Standard class typedefs. */
    typedef AgricPractMergeDataExtractionFiles        Self;
    typedef Application                   Superclass;
    typedef itk::SmartPointer<Self>       Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Standard macro */
    itkNewMacro(Self);

    itkTypeMacro(AgricPractMergeDataExtractionFiles, otb::Application);

    typedef std::map<std::string,int> MapIndex;//this map stores the index of data stored
                                               //in VectorData mapped to a string

    typedef struct FidInfos {
        std::string date;
        std::string date2;
        double meanVal;
        double stdDevVal;
    } FidInfosType;

    typedef struct Fid {
        std::string fid;
        std::string name;
        std::vector<FidInfosType> infos;
    } FidType;

    typedef struct {
          bool operator() (const FidInfosType &i, const FidInfosType &j) {
              return ((i.date.compare(j.date)) < 0);
              //return (i.date < j.date);
          }
    } FidInfosComparator;




private:
    AgricPractMergeDataExtractionFiles()
    {
    }

    void DoInit() override
    {
        SetName("AgricPractMergeDataExtractionFiles");
        SetDescription("Computes statistics on a training polygon set.");

        // Documentation
        SetDocName("Polygon Class Statistics");
        SetDocLongDescription("TODO");
        SetDocLimitations("None");
        SetDocAuthors("OTB-Team");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Learning);


        AddParameter(ParameterType_StringList, "il", "Input files");
        SetParameterDescription("il", "Support list of images to be merged to output");

        AddParameter(ParameterType_OutputFilename, "out", "Output XML file");
        SetParameterDescription("out","Output file to store results (XML format)");

        AddRAMParameter();

        // Doc example parameter settings
        SetDocExampleParameterValue("il", "file1.xml file2.xml");
        SetDocExampleParameterValue("out","time_series.xml");

        //SetOfficialDocLink();
    }

    void DoUpdateParameters() override
    {

    }


    void DoExecute() override
    {
        const std::vector<std::string> &inFilePaths = this->GetParameterStringList("il");
        if(inFilePaths.size() == 0) {
            otbAppLogFATAL(<<"No image was given as input!");
        }

        std::vector<std::string>::const_iterator itFile;
        std::vector<FidType> resultFidList;
        MapIndex mapIndex;
        for (itFile = inFilePaths.begin(); itFile != inFilePaths.end(); ++itFile) {
            std::vector<FidType> curFids;
            if (ReadXmlFile(*itFile, curFids)) {
                MergeFids(curFids, resultFidList, mapIndex);
            }
        }

        WriteOutputFile(resultFidList);

    }

    bool ReadXmlFile(const std::string &filePath, std::vector<FidType> &retFids) {
        otbAppLogINFO("Reading file " << filePath);
        TiXmlDocument doc(filePath);
        if (!doc.LoadFile()) {
            otbAppLogWARNING("Loading file " << filePath << " failed");
            return false;
        }

        TiXmlHandle hDoc(const_cast<TiXmlDocument *>(&doc));
        auto rootElement = hDoc.FirstChildElement("fids").ToElement();

        if (!rootElement) {
            otbAppLogWARNING("Cannot find fids root element in file " << filePath);
            return false;
        }
        for (auto valuesEl = rootElement->FirstChildElement("fid"); valuesEl;
             valuesEl = valuesEl->NextSiblingElement("fid")) {
            FidType fid;
            fid.fid = GetAttribute(valuesEl, "id");
            fid.name = GetAttribute(valuesEl, "name");
            fid.infos = ReadFidInfos(valuesEl);
            retFids.emplace_back(fid);
        }

        otbAppLogINFO("Reading file " << filePath << " OK");

        return true;

    }

    std::vector<FidInfos> ReadFidInfos(const TiXmlElement *el) {
        std::vector<FidInfos> result;
        for (auto valuesEl = el->FirstChildElement("info"); valuesEl;
             valuesEl = valuesEl->NextSiblingElement("info")) {

            FidInfos fidInfo;
            fidInfo.date = GetAttribute(valuesEl, "date");
            fidInfo.date2 = GetAttribute(valuesEl, "date2");
            fidInfo.meanVal = std::atof(GetAttribute(valuesEl, "mean").c_str());
            fidInfo.stdDevVal = std::atof(GetAttribute(valuesEl, "stdev").c_str());
           result.emplace_back(fidInfo);
        }
        return result;
    }

    void MergeFids(const std::vector<FidType> &curFids, std::vector<FidType> &resultFidList, MapIndex &mapIndex)
    {
        otbAppLogINFO("Merging fields ...");
        std::vector<FidType>::const_iterator it;
        for(it = curFids.begin(); it != curFids.end(); ++it) {
            // compare by name and not id in order to make distinction between VV and VH
            const std::string &uid = (it->fid + it->name);
            int idxInResults = LookupString(uid, mapIndex);
            if (idxInResults >=0) {
                // merge the infos
                resultFidList[idxInResults].infos.insert(std::end(resultFidList[idxInResults].infos),
                                                         std::begin(it->infos), std::end(it->infos));
            } else {
                // if not already present, add the current fid info at the end of the result
                // as the data is to be inserted at back therefore index is size of vector before insertion
                mapIndex.insert(std::make_pair(uid, resultFidList.size()));
                resultFidList.push_back(*it);
            }
        }
    }

    void WriteOutputFile(std::vector<FidType> &resultFidList) {
        std::ofstream xmlFile;
        const std::string &outFilePath = this->GetParameterString("out");

        otbAppLogINFO("Writing results to file " << outFilePath);

        xmlFile.open(outFilePath, std::ios_base::trunc | std::ios_base::out);
        xmlFile << "<fids>\n";

        std::vector<FidType>::iterator fidIt;
        for (fidIt = resultFidList.begin(); fidIt != resultFidList.end();
             ++fidIt)
        {
            WriteEntriesToOutputFile(xmlFile, *fidIt);
        }
        xmlFile << "</fids>";
    }

   void  WriteEntriesToOutputFile(std::ofstream &outStream, FidType &fileFieldsInfos)
    {
        // now sort the fid infos
        std::sort (fileFieldsInfos.infos.begin(), fileFieldsInfos.infos.end(), m_FidInfosComparator);

        outStream << "  " << "<fid id=\"" << fileFieldsInfos.fid.c_str() << "\" name=\"" << fileFieldsInfos.name.c_str() << "\">\n";

        for (size_t i = 0; i<fileFieldsInfos.infos.size(); i++) {
            outStream << "    " << "<info date=\"" << fileFieldsInfos.infos[i].date.c_str() << "\" ";
            if (fileFieldsInfos.infos[i].date2.size() > 0) {
                outStream << "date2=\"" << fileFieldsInfos.infos[i].date2.c_str() << "\" ";
            }
//            outStream << "mean=\"" << (boost::lexical_cast<std::string>(fileFieldsInfos.infos[i].meanVal)).c_str() << "\" ";
//            outStream << "stdev=\"" << (boost::lexical_cast<std::string>(fileFieldsInfos.infos[i].stdDevVal)).c_str() << "\" ";
            outStream << "mean=\"" << (DoubleToString(fileFieldsInfos.infos[i].meanVal)).c_str() << "\" ";
            outStream << "stdev=\"" << (DoubleToString(fileFieldsInfos.infos[i].stdDevVal)).c_str() << "\" ";

            outStream << "/>\n";
        }
        outStream << "  " << "</fid>\n";
    }

   int LookupString(const std::string &s, const MapIndex &mapIndex) {
       MapIndex::const_iterator pt = mapIndex.find(s);
       if (pt != mapIndex.end()) {
           return pt->second;
       }
       return -1; // not found
   }

private:
    FidInfosComparator m_FidInfosComparator;

};

} // end of namespace Wrapper
} // end of namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::AgricPractMergeDataExtractionFiles)
