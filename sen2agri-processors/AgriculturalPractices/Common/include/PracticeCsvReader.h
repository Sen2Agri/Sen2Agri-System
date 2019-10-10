#ifndef PracticeCsvReader_h
#define PracticeCsvReader_h

#include <vector>
#include <fstream>

#include "PracticeReaderBase.h"

#define HEADER_SIZE                 11
#define HEADER_SIZE_WITH_SEQ_ID     22

class PracticeCsvReader : public PracticeReaderBase
{
public:
    virtual std::string GetName() {return "csv";}
    virtual bool ExtractFeatures(std::function<bool (const FeatureDescription&, void *payload)> fnc, void *payload=NULL);

private:
    class CsvFeatureDescription : public FeatureDescription
    {
        CsvFeatureDescription() : m_bIsValid(false) {
            m_SeqFieldIdFieldIdx = -1;
            m_S1PixIdx = -1;
            m_csvSeparator = ',';
        }
        virtual std::string GetFieldId() const;
        virtual std::string GetFieldSeqId() const;
        virtual std::string GetCountryCode() const;
        virtual std::string GetYear() const;
        virtual std::string GetMainCrop() const;
        virtual std::string GetVegetationStart() const;
        virtual std::string GetHarvestStart() const;
        virtual std::string GetHarvestEnd() const;
        virtual std::string GetPractice() const;
        virtual std::string GetPracticeType() const;
        virtual std::string GetPracticeStart() const;
        virtual std::string GetPracticeEnd() const;
        virtual std::string GetS1Pix() const;

    private:

        int GetPosInHeader(const std::string &item);
        bool ExtractLineInfos(const std::string &line);
        std::vector<std::string> LineToVector(const std::string &line, const char sep);
        bool ExtractHeaderInfos(const std::string &line);


        std::ifstream m_fStream;
        std::vector<std::string> m_InputFileHeader;

        int m_FieldIdFieldIdx;
        int m_SeqFieldIdFieldIdx;
        int m_CountryFieldIdx;
        int m_YearFieldIdx;
        int m_MainCropFieldIdx;
        int m_VegStartFieldIdx;
        int m_HarvestStartFieldIdx;
        int m_HarvestEndFieldIdx;
        int m_PracticeFieldIdx;
        int m_PracticeTypeFieldIdx;
        int m_PracticeStartFieldIdx;
        int m_PracticeEndFieldIdx;
        int m_S1PixIdx;

        std::string m_FieldIdVal;
        std::string m_SeqFieldIdVal;
        std::string m_CountryVal;
        std::string m_YearVal;
        std::string m_MainCropVal;
        std::string m_VegStartVal;
        std::string m_HarvestStartVal;
        std::string m_HarvestEndVal;
        std::string m_PracticeVal;
        std::string m_PracticeTypeVal;
        std::string m_PracticeStartVal;
        std::string m_PracticeEndVal;
        std::string m_S1Pix;

        bool m_bIsValid;
        std::string m_source;
        char m_csvSeparator;

        friend class PracticeCsvReader;
    };
};

#endif
