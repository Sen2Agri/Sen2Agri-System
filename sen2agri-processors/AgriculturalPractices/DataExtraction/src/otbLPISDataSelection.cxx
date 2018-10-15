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
#include <boost/algorithm/string.hpp>

#include "string_utils.hpp"
#include "otbOGRDataSourceWrapper.h"

#define CATCH_CROP_VAL                  "CatchCrop"
#define FALLOW_LAND_VAL                 "Fallow"
#define NITROGEN_FIXING_CROP_VAL        "NFC"

namespace otb
{
namespace Wrapper
{
class LPISDataSelection : public Application
{
public:
    /** Standard class typedefs. */
    typedef LPISDataSelection        Self;
    typedef Application                   Superclass;
    typedef itk::SmartPointer<Self>       Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Standard macro */
    itkNewMacro(Self);

    itkTypeMacro(LPISDataSelection, otb::Application);

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
    LPISDataSelection()
    {
    }

    void DoInit() override
    {
        SetName("LPISDataSelection");
        SetDescription("Computes statistics on a training polygon set.");

        // Documentation
        SetDocName("Polygon Class Statistics");
        SetDocLongDescription("TODO");
        SetDocLimitations("None");
        SetDocAuthors("OTB-Team");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Learning);


        AddParameter(ParameterType_String, "inshp", "Input shapefile");
        SetParameterDescription("inshp", "Support list of images to be merged to output");

        AddParameter(ParameterType_StringList, "addfiles", "Additional files");
        SetParameterDescription("addfiles", "TODO");
        MandatoryOff("addfiles");

        AddParameter(ParameterType_String, "practice", "Practice");
        SetParameterDescription("practice", "TODO");
        MandatoryOff("practice");

        AddParameter(ParameterType_String, "country", "Country");
        SetParameterDescription("country", "TODO");

        AddParameter(ParameterType_String, "year", "Year");
        SetParameterDescription("year", "TODO");

        AddParameter(ParameterType_String, "vegstart", "Vegetation start");
        SetParameterDescription("vegstart", "TODO");

        AddParameter(ParameterType_String, "hstart", "Harvest start");
        SetParameterDescription("hstart", "TODO");

        AddParameter(ParameterType_String, "hwinterstart", "Harvest winter start");
        SetParameterDescription("hwinterstart", "TODO");
        MandatoryOff("hwinterstart");

        AddParameter(ParameterType_String, "hend", "Harvest end");
        SetParameterDescription("hend", "TODO");

        AddParameter(ParameterType_String, "pstart", "Practice start");
        SetParameterDescription("pstart", "TODO");
        MandatoryOff("pstart");

        AddParameter(ParameterType_String, "pend", "Practice end");
        SetParameterDescription("pend", "TODO");
        MandatoryOff("pend");

        AddParameter(ParameterType_String, "wpstart", "Winter Practice start");
        SetParameterDescription("wpstart", "TODO");
        MandatoryOff("wpstart");

        AddParameter(ParameterType_String, "wpend", "Winter Practice end");
        SetParameterDescription("pend", "TODO");
        MandatoryOff("wpend");

        AddParameter(ParameterType_OutputFilename, "out", "Output practice CSV file");
        SetParameterDescription("out", "TODO");


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
        const std::string &inShpFile = this->GetParameterString("inshp");
        if(inShpFile.size() == 0) {
            otbAppLogFATAL(<<"No image was given as input!");
        }

        m_year = this->GetParameterString("year");
        m_outFileStream.open(GetParameterAsString("out"), std::ios_base::trunc | std::ios_base::out );
        WriteHeader();

        m_country = this->GetParameterString("country");
        auto factory = CountryInfoFactory::New();
        m_pCountryInfos = factory->GetCountryInfo(m_country);

        m_pCountryInfos->SetVegStart(GetParameterAsString("vegstart"));
        std::string practice;
        std::string practiceStart;
        std::string practiceEnd;
        std::string wPracticeStart;
        std::string wPracticeEnd;
        if (HasValue("practice")) {
            practice = GetParameterAsString("practice");
        }
        if (HasValue("pstart")) {
            practiceStart = GetParameterAsString("pstart");
        }
        if (HasValue("pend")) {
            practiceEnd = GetParameterAsString("pend");
        }
        if (HasValue("wpstart")) {
            wPracticeStart = GetParameterAsString("wpstart");
        }
        if (HasValue("wpend")) {
            wPracticeEnd = GetParameterAsString("wpend");
        }
        m_pCountryInfos->SetPractice(practice);
        m_pCountryInfos->SetPStart(practiceStart);
        m_pCountryInfos->SetPEnd(practiceEnd);
        m_pCountryInfos->SetWinterPStart(wPracticeStart);
        m_pCountryInfos->SetWinterPEnd(wPracticeEnd);
        m_pCountryInfos->SetHStart(GetParameterAsString("hstart"));
        m_pCountryInfos->SetHEnd(GetParameterAsString("hend"));
        if (HasValue("hwinterstart")) {
            m_pCountryInfos->SetHWinterStart(GetParameterAsString("hwinterstart"));
        }

        if (HasValue("addfiles")) {
            m_pCountryInfos->SetAdditionalFiles(GetParameterStringList("addfiles"));
        }

        otb::ogr::DataSource::Pointer source = otb::ogr::DataSource::New(
            inShpFile, otb::ogr::DataSource::Modes::Read);
        for (otb::ogr::DataSource::const_iterator lb=source->begin(), le=source->end(); lb != le; ++lb)
        {
            otb::ogr::Layer const& inputLayer = *lb;
            otb::ogr::Layer::const_iterator featIt = inputLayer.begin();
            for(; featIt!=inputLayer.end(); ++featIt)
            {
                ProcessFeature(*featIt);
            }
        }
    }

    void ProcessFeature(const ogr::Feature& feature) {
        OGRFeature &ogrFeat = feature.ogr();
        // If the current line has required practice
        bool bWriteLine = false;
        if (m_pCountryInfos->GetPractice().size() > 0 &&
                m_pCountryInfos->GetPractice() != "NA") {
            if (m_pCountryInfos->GetHasPractice(ogrFeat, m_pCountryInfos->GetPractice())) {
                bWriteLine = true;
            }

        } else if (!m_pCountryInfos->GetHasPractice(ogrFeat, CATCH_CROP_VAL) &&
                !m_pCountryInfos->GetHasPractice(ogrFeat, FALLOW_LAND_VAL) &&
                !m_pCountryInfos->GetHasPractice(ogrFeat, NITROGEN_FIXING_CROP_VAL)) {
            bWriteLine = true;
        }

        if (bWriteLine) {
            WriteLine(feature);
        }
    }

    void WriteHeader() {
        if (!m_outFileStream.is_open()) {
            std::cout << "Trying  to write header in a closed stream!" << std::endl;
            return;
        }
        // # create result csv file for harvest and EFA practice evaluation
        m_outFileStream << "FIELD_ID;COUNTRY;YEAR;MAIN_CROP;VEG_START;H_START;H_END;PRACTICE;P_TYPE;P_START;P_END\n";
    }

    void WriteLine(const ogr::Feature& feature) {
        if (!m_outFileStream.is_open()) {
            std::cout << "Trying  to write feature in a closed stream!" << std::endl;
            return;
        }
        OGRFeature &ogrFeat = feature.ogr();
        m_outFileStream << m_pCountryInfos->GetUniqueId(ogrFeat) << ";" << m_country << ";" << m_year << ";";
        m_outFileStream << GetValueOrNA(m_pCountryInfos->GetMainCrop(ogrFeat)) << ";";
        m_outFileStream << GetValueOrNA(m_pCountryInfos->GetVegStart()) << ";";
        m_outFileStream << GetValueOrNA(m_pCountryInfos->GetHStart(ogrFeat)) << ";";
        m_outFileStream << GetValueOrNA(m_pCountryInfos->GetHEnd(ogrFeat)) << ";";
        m_outFileStream << GetValueOrNA(m_pCountryInfos->GetPractice(ogrFeat)) << ";";
        m_outFileStream << GetValueOrNA(m_pCountryInfos->GetPracticeType(ogrFeat)) << ";";
        m_outFileStream << GetValueOrNA(m_pCountryInfos->GetPStart(ogrFeat)) << ";";
        m_outFileStream << GetValueOrNA(m_pCountryInfos->GetPEnd(ogrFeat)) << "\n";
    }

    std::string GetValueOrNA(const std::string &str) {
        if (str.size() > 0) {
            return str;
        }
        return "NA";
    }

private:
    std::string m_year;

// //////////////////////////////////

private:

    typedef std::map<std::string, size_t> MapHdrIdx;

    class CountryInfoBase {
    public:
        virtual void SetAdditionalFiles(const std::vector<std::string> &additionalFiles) {
            if (m_additionalFiles.size() > 0 && m_LineHandlerFnc == nullptr) {
                std::cout << "ERROR: Additional files provided but no handler function defined for this country" << std::endl;
                return;
            }
            m_additionalFiles.insert(m_additionalFiles.end(), additionalFiles.begin(), additionalFiles.end());
            for(const auto &file: m_additionalFiles) {
                ParseCsvFile(file, m_LineHandlerFnc);
            }
        }
        virtual std::string GetName() = 0;
        virtual std::string GetUniqueId(OGRFeature &ogrFeat) = 0;
        virtual std::string GetMainCrop(OGRFeature &ogrFeat) = 0;
        virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice) = 0;

        virtual void SetYear(const std::string &val) { m_year = val;}
        virtual void SetVegStart(const std::string &val) { m_vegstart = val;}
        virtual void SetHStart(const std::string &val) { m_hstart = val;}
        virtual void SetHWinterStart(const std::string &val) { m_hWinterStart = val;}
        virtual void SetHEnd(const std::string &val) { m_hend = val;}
        virtual void SetPractice(const std::string &val) { m_practice = val;}
        virtual void SetPStart(const std::string &val) { m_pstart = val;}
        virtual void SetPEnd(const std::string &val) { m_pend = val;}
        virtual void SetWinterPStart(const std::string &val) { m_pWinterStart = val;}
        virtual void SetWinterPEnd(const std::string &val) { m_pWinterEnd = val;}

        virtual std::string GetYear() {return m_year;}
        virtual std::string GetVegStart() {return m_vegstart;}
        virtual std::string GetHStart(OGRFeature &ogrFeat) {(void)ogrFeat ; return m_hstart;}
        virtual std::string GetHEnd(OGRFeature &ogrFeat) {(void)ogrFeat ; return m_hend;}
        virtual std::string GetPractice() {return m_practice;}
        virtual std::string GetPractice(OGRFeature &ogrFeat) { (void)ogrFeat ; return GetPractice(); }
        virtual std::string GetPracticeType(OGRFeature &ogrFeat) {(void)ogrFeat ; return m_ptype;}
        virtual std::string GetPStart(OGRFeature &ogrFeat) {(void)ogrFeat ; return m_pstart;}
        virtual std::string GetPEnd(OGRFeature &ogrFeat) {(void)ogrFeat ; return m_pend;}

    private:

        void ParseCsvFile(const std::string &filePath,
                          std::function<int(const MapHdrIdx&, const std::vector<std::string>&, int)> fnc) {
            std::cout << "Loading CSV file " << filePath << std::endl;

            std::ifstream fileStream(filePath);
            if (!fileStream.is_open()) {
                std::cout << "ERROR: cannot open CSV file " << filePath << std::endl;
                return;
            }
            std::ptrdiff_t pos = std::find(m_additionalFiles.begin(), m_additionalFiles.end(), filePath) - m_additionalFiles.begin();
            std::string line;
            size_t curLine = 0;
            MapHdrIdx headerLineMap;
            while (std::getline(fileStream, line))
            {
                // first line should be the header
                std::vector<std::string> lineItems = GetInputFileLineElements(line);
                if (curLine++ == 0) {
                    for(size_t i = 0; i<lineItems.size(); i++) {
                        headerLineMap[lineItems[i]] = i;
                    }
                    continue;
                }
                if (lineItems.size() == headerLineMap.size()) {
                    fnc(headerLineMap, lineItems, pos);
                }
            }
        }

    protected:
        std::vector<std::string> GetInputFileLineElements(const std::string &line) {
            std::vector<std::string> results;
            boost::algorithm::split(results, line, [](char c){return c == ';';});
            return results;
        }

    protected:
        std::string m_year;
        std::string m_vegstart;
        std::string m_hstart;
        std::string m_hend;
        std::string m_hWinterStart;
        std::string m_practice;
        std::string m_pstart;
        std::string m_pend;
        std::string m_pWinterStart;
        std::string m_pWinterEnd;
        std::string m_ptype;

        std::vector<std::string> m_additionalFiles;

        std::function<int(const MapHdrIdx&, const std::vector<std::string>&, int)> m_LineHandlerFnc;
    };

    // //////////// CZE Infos ///////////////////
    class CzeCountryInfo : public CountryInfoBase {
    public:
        CzeCountryInfo() {
            using namespace std::placeholders;
            m_LineHandlerFnc = std::bind(&CzeCountryInfo::HandleFileLine, this, _1, _2, _3);
        }

        virtual std::string GetName() { return "CZE"; }
        virtual std::string GetUniqueId(OGRFeature &ogrFeat) {
            return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("NKOD_DPB"));
        }
        virtual std::string GetMainCrop(OGRFeature &ogrFeat) {
            return GetLpisInfos(GetUniqueId(ogrFeat)).plod1;
        }
        virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice) {
            bool bCheckVymera = false;
            const std::string &uid = GetUniqueId(ogrFeat);
            const EfaInfosType &efaInfos = GetEfaInfos(uid);
            const std::string &typEfa = efaInfos.typ_efa;
            if (practice == CATCH_CROP_VAL && typEfa == "MPL") {
                bCheckVymera = true;
            } else if (practice == NITROGEN_FIXING_CROP_VAL && typEfa == "PVN") {
                bCheckVymera = true;
            } else if (practice == FALLOW_LAND_VAL && typEfa == "UHOZ") {
                return true;
            }
            if (bCheckVymera) {
                const LpisInfosType &lpisInfos = GetLpisInfos(uid);
                if (lpisInfos.vymera == efaInfos.vym_efa) {
                    return true;
                }
            }
            return false;
        }

        virtual std::string GetPracticeType(OGRFeature &ogrFeat) {
            if (m_practice == CATCH_CROP_VAL) {
                const std::string &mpl = GetEfaInfos(GetUniqueId(ogrFeat)).var_mpl;
                if (mpl == "L") {
                    return "Summer";
                }
                return "Winter";
            }
            return "NA";
        }
        virtual std::string GetPStart(OGRFeature &ogrFeat) {
            if (m_practice == CATCH_CROP_VAL) {
                const std::string &pType = GetPracticeType(ogrFeat);
                if (pType == "Winter") {
                    return m_pWinterStart;
                } else if (pType == "Summer") {
                    return m_pstart;
                }
            } else if (m_practice == NITROGEN_FIXING_CROP_VAL) {
                return m_pstart;
            } else if (m_practice == FALLOW_LAND_VAL) {
                return m_pstart;
            }
            return "NA";
        }
        virtual std::string GetPEnd(OGRFeature &ogrFeat) {
            if (m_practice == CATCH_CROP_VAL) {
                const std::string &pType = GetPracticeType(ogrFeat);
                if (pType == "Winter") {
                    return m_pWinterEnd;
                } else if (pType == "Summer") {
                    return m_pend;
                }
            } else if (m_practice == NITROGEN_FIXING_CROP_VAL || m_practice == FALLOW_LAND_VAL) {
                return m_pend;
            }
            return "NA";

        }

    private :
        typedef struct {
            std::string plod1;
            std::string vymera;
        } LpisInfosType;

        typedef struct {
            std::string typ_efa;
            std::string vym_efa;
            std::string var_mpl;
        } EfaInfosType;

        std::map<std::string, EfaInfosType> efaInfosMap;
        std::map<std::string, LpisInfosType> lpisInfosMap;

        int HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx) {
            MapHdrIdx::const_iterator itMap;
            LpisInfosType lpisInfos;
            EfaInfosType efaInfos;
            switch (fileIdx) {
                case 0:     //LPIS file
                    itMap = header.find("PLOD1");
                    if (itMap != header.end() && itMap->second < line.size()) {
                        lpisInfos.plod1 = line[itMap->second];
                    }
                    itMap = header.find("VYMERA");
                    if (itMap != header.end() && itMap->second < line.size()) {
                        lpisInfos.vymera = line[itMap->second];
                        std::replace( lpisInfos.vymera.begin(), lpisInfos.vymera.end(), ',', '.');
                    }
                    itMap = header.find("NKOD_DPB");
                    if (itMap != header.end() && itMap->second < line.size()) {
                        lpisInfosMap[line[itMap->second]] = lpisInfos;
                    }
                    break;
                case 1:         // EFA file
                    itMap = header.find("TYP_EFA");
                    if (itMap != header.end() && itMap->second < line.size()) {
                        efaInfos.typ_efa = line[itMap->second];
                    }
                    itMap = header.find("VYM_EFA");
                    if (itMap != header.end() && itMap->second < line.size()) {
                        efaInfos.vym_efa = line[itMap->second];
                        std::replace( efaInfos.vym_efa.begin(), efaInfos.vym_efa.end(), ',', '.');
                    }

                    itMap = header.find("VAR_MPL");
                    if (itMap != header.end() && itMap->second < line.size()) {
                        efaInfos.var_mpl = line[itMap->second];
                    }

                    itMap = header.find("NKOD_DPB");
                    if (itMap != header.end() && itMap->second < line.size()) {
                        efaInfosMap[line[itMap->second]] = efaInfos;
                    }
                    break;
                default:
                    return false;
            }
            return true;
        }

        LpisInfosType GetLpisInfos(const std::string &fid) {
            std::map<std::string, LpisInfosType>::const_iterator itMap = lpisInfosMap.find(fid);
            if (itMap != lpisInfosMap.end()) {
                return itMap->second;
            }
            return LpisInfosType();
        }

        EfaInfosType GetEfaInfos(const std::string &fid) {
            std::map<std::string, EfaInfosType>::const_iterator itMap = efaInfosMap.find(fid);
            if (itMap != efaInfosMap.end()) {
                return itMap->second;
            }
            return EfaInfosType();
        }


    };

    // //////////// NL Infos ///////////////////
    class NlCountryInfo  : public CountryInfoBase {
    public:
        virtual std::string GetName() { return "NL"; }
        virtual std::string GetUniqueId(OGRFeature &ogrFeat) {
            return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("FUNCTIONEE"));
        }
        virtual std::string GetMainCrop(OGRFeature &ogrFeat) {
            return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("GRONDBEDEK"));
        }
        virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice) {
            if (practice == CATCH_CROP_VAL) {
                std::string field = ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("IND_EA"));
                if (field == "J") {
                    return true;
                }
                field = ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("GRONDBED_2"));
                if (field.size() > 0) {
                    return true;
                }
            }

            return false;
        }

        virtual std::string GetHStart(OGRFeature &ogrFeat) {
            const std::string &mainCrop = GetMainCrop(ogrFeat);
            if (mainCrop == "233") {
                return m_hWinterStart;
            }
            return m_hstart;
        }

        virtual std::string GetPractice(OGRFeature &ogrFeat) {
            if (m_practice == CATCH_CROP_VAL) {
                std::string field = ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("IND_EA"));
                if (field == "J") {
                    return "CatchCropIsMain";
                }
                field = ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("GRONDBED_2"));
                if (field.size() > 0) {
                    return m_practice;
                }
                return "NA";
            }
            // NO NFC or Fallow for NLD

            return m_practice;
        }

        virtual std::string GetPracticeType(OGRFeature &ogrFeat) {
            if (m_practice == CATCH_CROP_VAL) {
                std::string field = ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("GRONDBED_2"));
                std::string retPtype = field;
                if (field.size() > 0) {
                    retPtype = m_practice + "_" + std::to_string(std::atoi(field.c_str()));
                }
                const std::string &practice = GetPractice(ogrFeat);
                if (practice == "CatchCropIsMain") {
                    retPtype = "CatchCropIsMain" + retPtype;
                }
                return retPtype;
            }
            return "NA";
        }
        virtual std::string GetPStart(OGRFeature &ogrFeat) {
            if (m_practice == CATCH_CROP_VAL) {
                const std::string &practice = GetPractice(ogrFeat);
                if (practice == "CatchCropIsMain") {
                    return m_pstart;
                } else if (practice != "NA") {
                    return m_pWinterStart;
                }
            }
            return "NA";
        }
        virtual std::string GetPEnd(OGRFeature &ogrFeat) {
            if (m_practice == CATCH_CROP_VAL) {
                const std::string &practice = GetPractice(ogrFeat);
                if (practice == "CatchCropIsMain") {
                    return m_pend;
                } else {
                    return "NA";
                }
            }
            return "NA";

        }


    };

    // //////////// LTU Infos ///////////////////
    class LtuCountryInfo : public CountryInfoBase {
    private :
        std::map<std::string, std::string> m_ccISMap;
        std::map<std::string, std::string> m_ccPOMap;
        std::map<std::string, std::string> m_greenFallowMap;
        std::map<std::string, std::string> m_blackFallowMap;
        std::map<std::string, std::string> m_nfcMap;

    public:
        LtuCountryInfo() {
            using namespace std::placeholders;
            m_LineHandlerFnc = std::bind(&LtuCountryInfo::HandleFileLine, this, _1, _2, _3);
        }

        virtual std::string GetName() { return "LTU"; }
        virtual std::string GetUniqueId(OGRFeature &ogrFeat) {
            return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("agg_id"));
        }

        virtual std::string GetMainCrop(OGRFeature &ogrFeat) {
            return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("PSL_KODAS"));
        }
        virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice) {
            const std::string &uid = GetUniqueId(ogrFeat);
            if (practice == CATCH_CROP_VAL) {
                return (m_ccISMap.find(uid) != m_ccISMap.end() ||
                        m_ccPOMap.find(uid) != m_ccPOMap.end());
            } else if (practice == FALLOW_LAND_VAL) {
                return (m_blackFallowMap.find(uid) != m_blackFallowMap.end() ||
                        m_greenFallowMap.find(uid) != m_greenFallowMap.end());
            } else if (practice == NITROGEN_FIXING_CROP_VAL) {
                return (m_nfcMap.find(uid) != m_nfcMap.end());
            }
            return false;
        }

        virtual std::string GetPracticeType(OGRFeature &ogrFeat) {
            const std::string &uid = GetUniqueId(ogrFeat);
            if (m_practice == CATCH_CROP_VAL) {
                if (m_ccISMap.find(uid) != m_ccISMap.end()) {
                    return "IS";
                } else if (m_ccPOMap.find(uid) != m_ccPOMap.end()) {
                    return "PO";
                }
            } else if (m_practice == FALLOW_LAND_VAL) {
                if (m_blackFallowMap.find(uid) != m_blackFallowMap.end()) {
                    return "PDJ";
                }
                if(m_greenFallowMap.find(uid) != m_greenFallowMap.end()){
                    return "PDZ";
                }
            } // else if (practice == NITROGEN_FIXING_CROP_VAL) return "NA"
            return "NA";
        }

        virtual std::string GetPEnd(OGRFeature &ogrFeat) {
            if (m_practice == FALLOW_LAND_VAL) {
                const std::string &pType = GetPracticeType(ogrFeat);
                if (pType == "PDZ") {
                    return m_pWinterEnd;
                }
            }
            return m_pend;
        }

        int HandleFileLine(const MapHdrIdx& header, const std::vector<std::string>& line, int fileIdx) {
            std::map<std::string, std::string> *pRefMap = NULL;
            switch(fileIdx) {
            case 0:
                if (m_practice.size() == 0 || m_practice == CATCH_CROP_VAL) {
                    // we have a ccPO file
                    pRefMap = &m_ccPOMap;
                } else if (m_practice == FALLOW_LAND_VAL) {
                    pRefMap = &m_blackFallowMap;
                } else if (m_practice == NITROGEN_FIXING_CROP_VAL) {
                    pRefMap = &m_nfcMap;
                }

                break;
            case 1:
                if (m_practice.size() == 0 || m_practice == CATCH_CROP_VAL) {
                    // we have a ccIS file
                    pRefMap = &m_ccISMap;
                } else if (m_practice == FALLOW_LAND_VAL) {
                    pRefMap = &m_greenFallowMap;
                } else {
                    std::cout << "ERROR: Unexpected file here!!!" << std::endl;
                    return false;
                }
                break;
            case 2:
                // here is expected only for no practice selections
                pRefMap = &m_blackFallowMap;
                break;
            case 3:
                // here is expected only for no practice selections
                pRefMap = &m_greenFallowMap;
                break;
            case 4:
                // here is expected only for no practice selections
                pRefMap = &m_nfcMap;
                break;
            }

            if (pRefMap) {
                std::string uid;
                std::vector<std::string> keys = {"valdos numeris", "kZs", "deklaruoto lauko numeris"};
                for(size_t i = 0; i<keys.size();i++) {
                    MapHdrIdx::const_iterator itMap = header.find(keys[i]);
                    if (itMap != header.end() && itMap->second < line.size()) {
                        if (i != 0) {
                            uid += "-";
                        }
                        uid += line[itMap->second];
                    }
                }
                (*pRefMap)[uid] = uid;
            } else {
                std::cout << "An error occurred trying to identify the file!!!" << std::endl;
                return false;
            }
            return true;
        }

        bool HasUid(const std::string &fid, const std::map<std::string, std::string> &refMap) {
            std::map<std::string, std::string>::const_iterator itMap = refMap.find(fid);
            return itMap != refMap.end();
        }
    };

    // //////////// ROMANIA Infos ///////////////////
    class RouCountryInfo : public CountryInfoBase {
    public:
        virtual std::string GetName() { return "ROU"; }
        virtual std::string GetUniqueId(OGRFeature &ogrFeat) {
            return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("TODO"));
        }
        virtual std::string GetMainCrop(OGRFeature &ogrFeat) {
            (void)ogrFeat;
            return "TODO";
        }
        virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice) {
            (void)ogrFeat;
            (void)practice;
            return false;
        }

    };

    // //////////// ITALY Infos ///////////////////
    class ItaCountryInfo : public CountryInfoBase {
    public:
        virtual std::string GetName() { return "ITA"; }
        virtual std::string GetUniqueId(OGRFeature &ogrFeat) {
            return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("TODO"));
        }
        virtual std::string GetMainCrop(OGRFeature &ogrFeat) {
            (void)ogrFeat;
            return "TODO";
        }
        virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice) {
            (void)ogrFeat;
            (void)practice;
            return false;
        }

    };

    // //////////// SPAIN Infos ///////////////////
    class EspCountryInfo : public CountryInfoBase {
    private:
        const std::map<int, int> m_nflCropCodes = {{34,   34}, {40,   40}, {41,   41}, {43,   43}, {49,   49}, {50,   50}, {51,   51}, {52,   52}, {53,   53}, {60,   60},
    {61,   61}, {67,   67}, {76,   76}, {87,   87}, {180,  180}, {238,  238}, {239,  239}, {240,  240}, {248,  248}, {249,  249},
    {250,  250}, {77,   77 }, {241,  241}, {242,  242}, {243,  243}, {244,  244}, {245,  245}, {246,  246}, {298,  298}, {299,  299},
    {336,  336}, {337,  337}, {338,  338}, {339,  339}, {340,  340}, {342,   342}};
    public:
        virtual std::string GetName() { return "ESP"; }

        virtual std::string GetUniqueId(OGRFeature &ogrFeat) {
            return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_DECLARA"));
        }
        virtual std::string GetMainCrop(OGRFeature &ogrFeat) {
            return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_PRODUCTO"));
        }
        virtual bool GetHasPractice(OGRFeature &ogrFeat, const std::string &practice) {
            if (practice == FALLOW_LAND_VAL) {
                int cropCode = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_PRODUCTO")));
                int variedad = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_VARIEDAD")));
                if (cropCode == 20 || cropCode == 23) {
                    if (variedad == 901 || variedad == 902) {
                        return true;
                    }
                } else if (cropCode == 21 || cropCode == 25 || cropCode == 334) {
                    if (variedad == 0) {
                        return true;
                    }
                }
            } else if (practice == NITROGEN_FIXING_CROP_VAL) {
                 int cropCode = std::atoi(ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_PRODUCTO")));
                 if (m_nflCropCodes.find(cropCode) != m_nflCropCodes.end()) {
                     return true;
                 }
            }
            return false;
        }

        virtual std::string GetPracticeType(OGRFeature &ogrFeat) {
            if (m_practice == FALLOW_LAND_VAL) {
                return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex("C_VARIEDAD"));
            } // else if (practice == NITROGEN_FIXING_CROP_VAL) return "NA"
            return "NA";
        }

    };

    class CountryInfoFactory : public itk::LightObject
    {
    public:
        typedef CountryInfoFactory Self;
        typedef itk::LightObject Superclass;
        typedef itk::SmartPointer<Self> Pointer;
        typedef itk::SmartPointer<const Self> ConstPointer;

        itkNewMacro(Self)

        itkTypeMacro(CountryInfoFactory, itk::LightObject)

        inline std::unique_ptr<CountryInfoBase> GetCountryInfo(const std::string &name) {
            std::unique_ptr<CountryInfoBase> czeInfos(new CzeCountryInfo);
            if (czeInfos->GetName() == name) {
                return czeInfos;
            }
            std::unique_ptr<CountryInfoBase> nlInfos(new NlCountryInfo);
            if (nlInfos->GetName() == name) {
                return nlInfos;
            }
            std::unique_ptr<CountryInfoBase> ltuInfos(new LtuCountryInfo);
            if (ltuInfos->GetName() == name) {
                return ltuInfos;
            }
            std::unique_ptr<CountryInfoBase> rouInfos(new RouCountryInfo);
            if (rouInfos->GetName() == name) {
                return rouInfos;
            }
            std::unique_ptr<CountryInfoBase> itaInfos(new ItaCountryInfo);
            if (itaInfos->GetName() == name) {
                return itaInfos;
            }
            std::unique_ptr<CountryInfoBase> espInfos(new EspCountryInfo);
            if (espInfos->GetName() == name) {
                return espInfos;
            }

            itkExceptionMacro("Practice reader not supported: " << name);

            return NULL;
        }
    };


private:
    std::string m_country;
    std::string m_practice;
    std::unique_ptr<CountryInfoBase> m_pCountryInfos;
    std::ofstream m_outFileStream;
};

} // end of namespace Wrapper
} // end of namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::LPISDataSelection)
