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

#include <fstream>
#include <iostream>
#include <expat.h>

#include "tinyxml_utils.hpp"
#include "string_utils.hpp"
#include "CommonFunctions.h"
#include <inttypes.h>
#include <boost/algorithm/string.hpp>

#define BUFFSIZE 2048*2048

// fid;suffix;date;mean;stdev
#define SIMPLE_HEADER_SIZE  5
// fid;suffix;date1;date2;mean;stdev
#define COHE_HEADER_SIZE    6

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
        void Reset() {
            fid.clear();
            name.clear();
            infos.clear();
        }
    } FidType;

    typedef struct {
          bool operator() (const FidInfosType &i, const FidInfosType &j) {
              return ((i.date.compare(j.date)) < 0);
              //return (i.date < j.date);
          }
    } FidInfosComparator;

    typedef struct XmlFieldInfos {
        XmlFieldInfos() {
            fidStarted = false;
        }
        FidType curFid;
        std::vector<FidType> extractedFids;
        bool fidStarted;
        std::string suffixFilter;

    } XmlFieldInfos;

    static void startFn(void *pUserData, const char* pElement, const char** pAttributes)
    {
        XmlFieldInfos* infos = (XmlFieldInfos*)pUserData;
        if (strcmp(pElement, "fid") == 0) {
            // we have a fid starting
            infos->curFid.Reset();
            infos->fidStarted = false;

            for( int i = 0; pAttributes[i] != NULL; i+=2) {
                if (strcmp(pAttributes[i], "id") == 0) {
                    infos->curFid.fid = pAttributes[i+1];
                } else if (strcmp(pAttributes[i], "name") == 0) {
                    infos->curFid.name = pAttributes[i+1];
                }
            }
            if (infos->curFid.fid.size() && infos->curFid.name.size()) {
                // check if the fid passes the filtering
                if (infos->suffixFilter.length() == 0 ||
                        infos->curFid.name.find(infos->suffixFilter) != std::string::npos) {
                    infos->fidStarted = true;
                }
            }
        } else if (infos->fidStarted && strcmp(pElement, "info") == 0) {
            FidInfosType fidInfos;

            for( int i = 0; pAttributes[i] != NULL; i+=2) {
                if (strcmp(pAttributes[i], "date") == 0) {
                    fidInfos.date = pAttributes[i+1];
                } else if (strcmp(pAttributes[i], "mean") == 0) {
                    fidInfos.meanVal = std::atof(pAttributes[i+1]);
                } else if (strcmp(pAttributes[i], "stdev") == 0) {
                    fidInfos.stdDevVal = std::atof(pAttributes[i+1]);
                } else if (strcmp(pAttributes[i], "date2") == 0) {
                    fidInfos.date2 = pAttributes[i+1];
                }
            }
            if (fidInfos.date.size()) {
                infos->curFid.infos.push_back(fidInfos);
            }
        }
    }

    static void endFn(void *pUserData, const char* pElement)
    {
        XmlFieldInfos* infos = (XmlFieldInfos*)pUserData;
        if (strcmp(pElement, "fid") == 0) {
            if (infos->fidStarted && infos->curFid.fid.size()) {
                infos->extractedFids.push_back(infos->curFid);
            }
            infos->curFid.Reset();
            infos->fidStarted = false;
        }
    }

    static void characterFn( void *pUserData, const char* pCharacterData, int length )
    {
        (void)pUserData;
        (void)pCharacterData;
        (void)length;
    }

private:
    AgricPractMergeDataExtractionFiles() : m_bForceKeepSuffixInOutput(false)
    {
        m_HeaderFields = {"KOD_PB", "date", "mean", "stdev"};
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
        SetParameterDescription("il", "Support list of files to be merged to output. "
                                      "If it is a directory, then all csv files from it will be used");

        AddParameter(ParameterType_OutputFilename, "out", "Output file (CSV or XML)");
        SetParameterDescription("out","Output file to store results. If the path is a folder and CSV is specified "
                                      "as format, then there is created a file for each parcel (with the specified limit)");

        AddParameter(ParameterType_String, "outformat", "The format of the output (csv or xml)");
        SetParameterDescription("outformat","The format of the output (csv or xml)");
        MandatoryOff("outformat");

        AddParameter(ParameterType_String, "sfilter", "Suffix filter");
        SetParameterDescription("sfilter","If this filter is set, then only items that will contain in suffix"
                                            "this filter will be merged");
        MandatoryOff("sfilter");

        AddParameter(ParameterType_Int, "skeep", "Force keeping the suffix in the output file");
        SetParameterDescription("skeep","If the output is a folder and individual files are created, force"
                                            "keeping the suffix in the output files (for coherence, for ex.)");
        MandatoryOff("skeep");
        SetDefaultParameterInt("skeep",0);

        AddParameter(ParameterType_Int, "csvcompact", "Wite the CSV file compacted.");
        SetParameterDescription("csvcompact", "The entries for a field are written on the same line "
                                              "separated by |, without duplicating fid and suffix.");
        MandatoryOff("csvcompact");
        SetDefaultParameterInt("csvcompact",1);

        AddParameter(ParameterType_Int, "limit", "The limit of the individual CSV files (if folder specified as out)");
        SetParameterDescription("limit","The limit of the individual CSV files (if folder specified as out and "
                                            "format is CSV)");
        MandatoryOff("limit");
        SetDefaultParameterInt("limit",5000);

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
        const std::vector<std::string> &inFilePaths = GetInputFilePaths();
        if (HasValue("sfilter")) {
            m_suffixFilter = GetParameterAsString("sfilter");
        }

        if (HasValue("skeep")) {
            m_bForceKeepSuffixInOutput = (GetParameterInt("skeep") != 0);
        }
        if (HasValue("outformat")) {
            m_outFormat = GetParameterAsString("outformat");
        } else {
            // determine the format from the input files
            boost::filesystem::path pathObj(inFilePaths[0]);
            std::string ext = pathObj.extension().string();
            if (boost::iequals(ext, ".xml")) {
                m_outFormat = "xml";
            } else if (boost::iequals(ext, ".csv")) {
                m_outFormat = "csv";
            } else {
                otbAppLogFATAL(<<"Invalid extension of input files " << ext);
            }
        }
        m_bCsvCompactMode = (GetParameterInt("csvcompact") != 0);

        std::vector<std::string>::const_iterator itFile;
        std::vector<FidType> resultFidList;
        MapIndex mapIndex;
        int i = 1;
        for (itFile = inFilePaths.begin(); itFile != inFilePaths.end(); ++itFile) {
            std::vector<FidType> curFids;
            if (ReadFile(*itFile, curFids)) {
                MergeFids(curFids, resultFidList, mapIndex);
            }
            otbAppLogINFO("File " << (*itFile) << " Done!");
            otbAppLogINFO("Processed " << i << " files. Remaining files = " << inFilePaths.size() - i <<
                          ". Percent completed = " << (int)((((double)i) / inFilePaths.size()) * 100) << "%");
            i++;

        }

        // Here we must remove all the duplicates
        RemoveDuplicates(resultFidList);

        WriteOutputFile(resultFidList);

    }

    bool ReadFile(const std::string &filePath, std::vector<FidType> &retFids) {
        otbAppLogINFO("Reading file " << filePath);
        boost::filesystem::path pathObj(filePath);
        if (boost::iequals(pathObj.extension().string(), ".xml")) {
            return ReadXmlFile(filePath, retFids);
        } else {
            return ReadCsvFile(filePath, retFids);
        }
    }

    bool ReadXmlFile(const std::string &filePath, std::vector<FidType> &retFids) {
        otbAppLogINFO("Reading file " << filePath);
        std::ifstream ifs( filePath.c_str() );
        if( ifs.fail() ) {
            otbAppLogFATAL("Error opening input file, exiting...");
            return false;
        }

        XML_Parser p = XML_ParserCreate(NULL);
        if (! p) {
            otbAppLogFATAL("Failed to create parser");
            return false;
        }

        XmlFieldInfos xmlFieldsInfos;
        xmlFieldsInfos.suffixFilter = m_suffixFilter;

        XML_SetUserData( p, &xmlFieldsInfos );
        XML_SetElementHandler(p, startFn, endFn);
        XML_SetCharacterDataHandler(p, characterFn);

        bool done = false;
        int len = 0;
        int totalCount = len;
        char buff[BUFFSIZE];
        while( !done ) {
            ifs.read( buff, BUFFSIZE );
            done = ( (len = ifs.gcount()) < BUFFSIZE);
            totalCount += len;
            if( ifs.bad() ) {
                otbAppLogFATAL("Error in read operation.");
                return false;
            }
            if (! XML_Parse(p, buff, len, done)) {
                otbAppLogFATAL("Parse error at line " << XML_GetCurrentLineNumber(p)
                    << " with " << XML_ErrorString(XML_GetErrorCode(p)));
                return false;
            }
        }
        retFids.insert(retFids.end(), xmlFieldsInfos.extractedFids.begin(), xmlFieldsInfos.extractedFids.end());

        return true;
    }

    bool ReadCsvFile(const std::string &filePath, std::vector<FidType> &retFids) {
        std::ifstream ifs( filePath.c_str() );
        if( ifs.fail() ) {
            otbAppLogFATAL("Error opening input file, exiting...");
            return false;
        }
        std::string line;
        int i = 0;
        int j;
        bool isCohe = false;
        int hdrDiff;
        bool fieldValid;
        while (std::getline(ifs, line)) {
            // skip the header
            if (i == 0) {
                std::vector<std::string> results;
                boost::algorithm::split(results, line, [](char c){return c == ';';});
                isCohe = (results.size() == COHE_HEADER_SIZE);
                i++;
                continue;
            }
            std::vector<std::string> entries;
            // check if we have the compacted version of the CVS file
            boost::algorithm::split(entries, line, [](char c){return c == '|';});
            j = 0;
            FidType fid;
            fieldValid = true;
            for (const auto &entry: entries) {
                FidInfos fidInfo;
                std::vector<std::string> results;
                boost::algorithm::split(results, entry, [](char c){return c == ';';});
                int offset = 0;
                if (j == 0) {
                    // check if the field passes the filter, if the filter is set
                    if (m_suffixFilter.length() > 0 && results[1].find(m_suffixFilter) == std::string::npos) {
                        fieldValid = false;
                        break;
                    }
                    // extract the field id
                    fid.fid = results[0];
                    fid.name = results[1];
                    offset = 2;
                    hdrDiff = 0;
                } else {
                    hdrDiff = 2;
                }

                if (isCohe) {
                    if (results.size() != (size_t)(COHE_HEADER_SIZE - hdrDiff)) {
                        std::cout << "Invalid entry length at line " << i << std::endl;
                        return false;
                    }
                } else {
                    if (results.size() != (size_t)(SIMPLE_HEADER_SIZE - hdrDiff)) {
                        std::cout << "Invalid entry length at line " << i << std::endl;
                        return false;
                    }
                }
                // now extract the date(s) and the values
                int curIdx = offset;
                fidInfo.date = results[curIdx++];
                if (isCohe) {
                    fidInfo.date2 = results[curIdx++];
                }
                fidInfo.meanVal = std::atof(results[curIdx++].c_str());
                fidInfo.stdDevVal = std::atof(results[curIdx++].c_str());
                fid.infos.emplace_back(fidInfo);
                j++;
            }
            if (fieldValid) {
                retFids.emplace_back(fid);
            }
            i++;
        }
        return true;
    }


//    bool ReadXmlFile(const std::string &filePath, std::vector<FidType> &retFids) {
//        otbAppLogINFO("Reading file " << filePath);
//        TiXmlDocument doc(filePath);
//        if (!doc.LoadFile()) {
//            otbAppLogWARNING("Loading file " << filePath << " failed");
//            return false;
//        }

//        TiXmlHandle hDoc(const_cast<TiXmlDocument *>(&doc));
//        auto rootElement = hDoc.FirstChildElement("fids").ToElement();

//        if (!rootElement) {
//            otbAppLogWARNING("Cannot find fids root element in file " << filePath);
//            return false;
//        }
//        for (auto valuesEl = rootElement->FirstChildElement("fid"); valuesEl;
//             valuesEl = valuesEl->NextSiblingElement("fid")) {
//            FidType fid;
//            fid.fid = GetAttribute(valuesEl, "id");
//            fid.name = GetAttribute(valuesEl, "name");
//            fid.infos = ReadFidInfos(valuesEl);
//            retFids.emplace_back(fid);
//        }

//        otbAppLogINFO("Reading file " << filePath << " OK");

//        return true;

//    }

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
            // do not add empty fields
            if (it->infos.size() == 0) {
                continue;
            }

            AddEntriesToResultList(*it, resultFidList, mapIndex);
        }
    }

    void AddEntriesToResultList(const FidType &fidInfo, std::vector<FidType> &resultFidList, MapIndex &mapIndex) {
        // compare by name and not id in order to make distinction between VV and VH
        const std::string &uid = (fidInfo.fid + fidInfo.name);
        int idxInResults = LookupString(uid, mapIndex);
        if (idxInResults >=0) {
            // merge the infos
            resultFidList[idxInResults].infos.insert(std::end(resultFidList[idxInResults].infos),
                                                     std::begin(fidInfo.infos), std::end(fidInfo.infos));
        } else {
            // if not already present, add the current fid info at the end of the result
            // as the data is to be inserted at back therefore index is size of vector before insertion
            mapIndex.insert(std::make_pair(uid, resultFidList.size()));
            resultFidList.push_back(fidInfo);
        }
    }

    void RemoveDuplicates(std::vector<FidType> &resultFidList) {
        for (auto &fidType: resultFidList) {
            auto comp = [] ( const FidInfosType& lhs, const FidInfosType& rhs ) {
                // TODO: For now we keep the ones that have different mean values
                return ((lhs.date == rhs.date) && (lhs.date2 == rhs.date2)/*&&
                        (lhs.meanVal == rhs.meanVal)*/);};

            auto pred = []( const FidInfosType& lhs, const FidInfosType& rhs ) {return ((lhs.date.compare(rhs.date)) < 0);};
            std::sort(fidType.infos.begin(), fidType.infos.end(),pred);
            auto last = std::unique(fidType.infos.begin(), fidType.infos.end(),comp);
            fidType.infos.erase(last, fidType.infos.end());
        }
    }

    void WriteOutputFile(std::vector<FidType> &resultFidList) {
        const std::string &outFilePath = this->GetParameterString("out");
        if (boost::filesystem::is_directory(outFilePath)) {
            if (m_outFormat == "csv") {

                // Disable the compact mode
                m_bCsvCompactMode = false;

                WriteMultiCsvFiles(outFilePath, resultFidList);
            } else {
                otbAppLogFATAL("Mode not supported: output as directory but output format not csv!");
            }
        } else {
            std::ofstream outFileStream;
            std::ofstream indexFileStream;
            CreateOutputStreams(outFilePath, outFileStream, indexFileStream);
            if (m_outFormat == "xml") {
                WriteXmlOutputFile(outFileStream, indexFileStream, resultFidList);
            } else {
                WriteCsvOutputFile(outFileStream, indexFileStream, resultFidList);
            }
        }
    }

    void WriteXmlOutputFile(std::ofstream &outStream, std::ofstream &outIdxStream, std::vector<FidType> &resultFidList) {
        uintmax_t curFileIdx = 0;

        const std::string &fidsStart("<fids>\n");
        curFileIdx += fidsStart.size();
        outStream << fidsStart.c_str();

        std::vector<FidType>::iterator fidIt;
        for (fidIt = resultFidList.begin(); fidIt != resultFidList.end();
             ++fidIt)
        {
            WriteEntriesToOutputXmlFile(outStream, outIdxStream, *fidIt, curFileIdx);
        }
        outStream << "</fids>";
    }

   void  WriteEntriesToOutputXmlFile(std::ofstream &outStream, std::ofstream &outIdxStream, FidType &fileFieldsInfos, uintmax_t &curFileIdx)
    {
        // now sort the fid infos
        std::sort (fileFieldsInfos.infos.begin(), fileFieldsInfos.infos.end(), m_FidInfosComparator);

        std::stringstream ss;
        ss << "  " << "<fid id=\"" << fileFieldsInfos.fid.c_str() << "\" name=\"" << fileFieldsInfos.name.c_str() << "\">\n";

        for (size_t i = 0; i<fileFieldsInfos.infos.size(); i++) {
            ss << "    " << "<info date=\"" << fileFieldsInfos.infos[i].date.c_str() << "\" ";
            if (fileFieldsInfos.infos[i].date2.size() > 0) {
                ss << "date2=\"" << fileFieldsInfos.infos[i].date2.c_str() << "\" ";
            }
//            outStream << "mean=\"" << (boost::lexical_cast<std::string>(fileFieldsInfos.infos[i].meanVal)).c_str() << "\" ";
//            outStream << "stdev=\"" << (boost::lexical_cast<std::string>(fileFieldsInfos.infos[i].stdDevVal)).c_str() << "\" ";
            ss << "mean=\"" << (DoubleToString(fileFieldsInfos.infos[i].meanVal)).c_str() << "\" ";
            ss << "stdev=\"" << (DoubleToString(fileFieldsInfos.infos[i].stdDevVal)).c_str() << "\" ";

            ss << "/>\n";
        }
        ss << "  " << "</fid>\n";
        const std::string &ssStr = ss.str();
        size_t byteToWrite = ssStr.size();
        if (outIdxStream.is_open()) {
            outIdxStream << fileFieldsInfos.fid.c_str() << ";" << fileFieldsInfos.name.c_str() << ";" << curFileIdx << ";" << byteToWrite <<"\n";
        }
        curFileIdx += byteToWrite;
        outStream << ssStr.c_str();
    }

   int LookupString(const std::string &s, const MapIndex &mapIndex) {
       MapIndex::const_iterator pt = mapIndex.find(s);
       if (pt != mapIndex.end()) {
           return pt->second;
       }
       return -1; // not found
   }

   void WriteCsvOutputFile(std::ofstream &outStream, std::ofstream &outIdxStream, std::vector<FidType> &resultFidList) {
       uintmax_t curFileIdx = 0;
       std::vector<FidType>::iterator fidIt;
       curFileIdx += WriteCsvHeader(outStream, resultFidList, false);
       for (fidIt = resultFidList.begin(); fidIt != resultFidList.end();
            ++fidIt)
       {
           WriteEntriesToOutputCsvFile(outStream, outIdxStream, *fidIt, curFileIdx, false);
       }
   }

  void  WriteEntriesToOutputCsvFile(std::ofstream &outStream, std::ofstream &outIdxStream, FidType &fileFieldsInfos, uintmax_t &curFileIdx,
                                    bool individualFieldFile)
   {
      std::stringstream ss;
      // now sort the fid infos
       std::sort (fileFieldsInfos.infos.begin(), fileFieldsInfos.infos.end(), m_FidInfosComparator);
       int fieldEntriesSize = fileFieldsInfos.infos.size();
       for (int i = 0; i<fieldEntriesSize; i++) {
           if (i == 0 || !m_bCsvCompactMode) {
               ss << fileFieldsInfos.fid.c_str() << ";";
               // write also the suffix if it is global file and no filter set
               if ((!individualFieldFile && m_suffixFilter.length() == 0) || m_bForceKeepSuffixInOutput) {
                   ss << fileFieldsInfos.name.c_str() << ";";
               }
           }
           ss << fileFieldsInfos.infos[i].date.c_str() << ";";
           if (fileFieldsInfos.infos[i].date2.length() != 0) {
               ss << fileFieldsInfos.infos[i].date2.c_str() << ";";
           }
           ss << DoubleToString(fileFieldsInfos.infos[i].meanVal).c_str() << ";" <<
                 DoubleToString(fileFieldsInfos.infos[i].stdDevVal).c_str();
           ss << (m_bCsvCompactMode ? ((i == fieldEntriesSize - 1) ? "\n" : "|") : "\n");
       }

       const std::string &ssStr = ss.str();
       size_t byteToWrite = ssStr.size();
       if (outIdxStream.is_open()) {
           outIdxStream << fileFieldsInfos.fid.c_str() << ";" << fileFieldsInfos.name.c_str() << ";" << curFileIdx << ";" << byteToWrite <<"\n";
       }
       curFileIdx += byteToWrite;
       outStream << ssStr.c_str();
   }

  void CreateOutputStreams(const std::string &outFilePath, std::ofstream &outFileStream, std::ofstream &indexFileStream) {
      std::string outIdxPath;
      boost::filesystem::path path(outFilePath);
      outIdxPath = (path.parent_path() / path.filename()).string() + ".idx";
      indexFileStream.open(outIdxPath, std::ios_base::trunc | std::ios_base::out);

      otbAppLogINFO("Writing results to file " << outFilePath);

      outFileStream.open(outFilePath, std::ios_base::trunc | std::ios_base::out);
  }

  void WriteMultiCsvFiles(const std::string &outDirPath, std::vector<FidType> &resultFidList) {
      // not used
      std::ofstream outIdxStream;
      uintmax_t curFileIdx = 0;
      (void)outIdxStream;
      (void)curFileIdx;

      std::vector<FidType>::iterator fidIt;
      int limit = GetParameterInt("limit");
      int filesCnt = 0;
      for (fidIt = resultFidList.begin(); fidIt != resultFidList.end(); ++fidIt)
      {
          std::string fileName = fidIt->fid;
          NormalizeFieldId(fileName);
          fileName = fileName + "_" + fidIt->name;
          const std::string &targetFile = GetIndividualFieldFileName(outDirPath, fileName);
          std::ofstream fileStream;
          fileStream.open(targetFile, std::ios_base::trunc | std::ios_base::out);
          if (!fileStream.is_open()) {
              otbAppLogWARNING("Cannot open the file " << targetFile);
              continue;
          }

          // write the header for the file
          WriteCsvHeader(fileStream, resultFidList, true);

          // write the entries in the file
          WriteEntriesToOutputCsvFile(fileStream, outIdxStream, *fidIt, curFileIdx, true);
          filesCnt++;
          if (filesCnt > limit) {
              otbAppLogWARNING("Limit of " << limit << " reached while writing individual files to folder " << outDirPath <<
                               ". No other files are written further");
              return;
          }
      }
  }

  int WriteCsvHeader(std::ofstream &fileStream, const std::vector<FidType> &resultFidList, bool individualFieldFile) {
      std::stringstream ss;
      for (size_t i = 0; i<m_HeaderFields.size(); i++) {
          ss << m_HeaderFields[i];
          if (i < m_HeaderFields.size()-1) {
              ss << ";";
          }
          if (resultFidList.size() > 0) {
              m_bUseDate2 = (resultFidList[0].infos[0].date2 != "");
              if (i == 0) {
                  if ((!individualFieldFile && m_suffixFilter.length() == 0) || m_bForceKeepSuffixInOutput) {
                      ss << "suffix" << ";";
                  }
              } else if (i == 1 && m_bUseDate2 != 0) {
                  ss << "date2" << ";";
              }
          }
      }
      ss << "\n";
      const std::string &ssStr = ss.str();
      fileStream << ssStr.c_str();
      return ssStr.size();
  }

  std::vector<std::string> GetInputFilePaths() {
      std::vector<std::string> retFilePaths;
      const std::vector<std::string> &inFilePaths = this->GetParameterStringList("il");
      for (const std::string &inPath: inFilePaths) {
          if ( !boost::filesystem::exists( inPath ) ) {
              otbAppLogWARNING("The provided input path does not exists: " << inPath);
              continue;
          }
          if (boost::filesystem::is_directory(inPath)) {
              boost::filesystem::directory_iterator end_itr;

              boost::filesystem::path dirPath(inPath);
              // cycle through the directory
              for (boost::filesystem::directory_iterator itr(dirPath); itr != end_itr; ++itr) {
                  if (boost::filesystem::is_regular_file(itr->path())) {
                      // assign current file name to current_file and echo it out to the console.
                      boost::filesystem::path pathObj = itr->path();
                      if (pathObj.has_extension()) {
                          std::string fileExt = pathObj.extension().string();
                          // Fetch the extension from path object and return
                          if (boost::iequals(fileExt, ".csv")) {
                              std::string current_file = pathObj.string();
                              if (std::find(retFilePaths.begin(), retFilePaths.end(), current_file) == retFilePaths.end()) {
                                  retFilePaths.push_back(current_file);
                              }
                          }
                      }
                  }
              }
          } else {
              if (std::find(retFilePaths.begin(), retFilePaths.end(), inPath) == retFilePaths.end()) {
                  retFilePaths.push_back(inPath);
              }
          }
      }

      if(retFilePaths.size() == 0) {
          otbAppLogFATAL(<<"No image was given as input!");
      }

      return retFilePaths;
  }


private:
    FidInfosComparator  m_FidInfosComparator;
    std::string         m_outFormat;
    bool m_bCsvCompactMode;

    std::vector<std::string> m_HeaderFields;
    bool m_bUseDate2;
    std::string m_suffixFilter;
    bool m_bForceKeepSuffixInOutput;

};

} // end of namespace Wrapper
} // end of namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::AgricPractMergeDataExtractionFiles)
