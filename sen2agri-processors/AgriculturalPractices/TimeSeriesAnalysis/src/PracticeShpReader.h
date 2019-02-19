#ifndef PracticeShpReader_h
#define PracticeShpReader_h

#include "otbOGRDataSourceWrapper.h"

#include "PracticeReaderBase.h"

class PracticeShpReader : public PracticeReaderBase
{
public:
    virtual std::string GetName() {return "shp";}
    virtual bool ExtractFeatures(std::function<bool (const FeatureDescription&)> fnc);

private:
    class OgrFeatureDescription : public FeatureDescription
    {
        OgrFeatureDescription() {
            m_FieldIdFieldName = "FIELD_ID";
            m_CountryFieldName = "COUNTRY";
            m_YearFieldName = "YEAR";
            m_MainCropFieldName = "MAIN_CROP";
            m_VegStartFieldName = "VEG_START";
            m_HarvestStartFieldName = "H_START";
            m_HarvestEndFieldName = "H_END";
            m_PracticeFieldName = "PRACTICE";
            m_PracticeTypeFieldName = "P_TYPE";
            m_PracticeStartFieldName = "P_START";
            m_PracticeEndFieldName = "P_END";
            m_S1PixFieldName = "S1Pix";
        }
        virtual std::string GetFieldId() const;
        virtual std::string GetFieldSeqId() const;
        virtual std::string GetMainCrop() const;
        virtual std::string GetCountryCode() const;
        virtual std::string GetYear() const;
        virtual std::string GetVegetationStart() const;
        virtual std::string GetHarvestStart() const;
        virtual std::string GetHarvestEnd() const;
        virtual std::string GetPractice() const;
        virtual std::string GetPracticeType() const;
        virtual std::string GetPracticeStart() const;
        virtual std::string GetPracticeEnd() const;
        virtual std::string GetS1Pix() const;
    private:
        otb::ogr::Layer::const_iterator featIt;

        std::string m_FieldIdFieldName;
        std::string m_CountryFieldName;
        std::string m_YearFieldName;
        std::string m_MainCropFieldName;
        std::string m_VegStartFieldName;
        std::string m_HarvestStartFieldName;
        std::string m_HarvestEndFieldName;
        std::string m_PracticeFieldName;
        std::string m_PracticeTypeFieldName;
        std::string m_PracticeStartFieldName;
        std::string m_PracticeEndFieldName;
        std::string m_S1PixFieldName;

        friend class PracticeShpReader;
    };
};

#endif
