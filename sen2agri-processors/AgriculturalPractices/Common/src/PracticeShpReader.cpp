#include "PracticeShpReader.h"
#include "otbOGRDataSourceWrapper.h"

bool PracticeShpReader::ExtractFeatures(std::function<bool (const FeatureDescription&, void *payload)> fnc, void *payload)
{
    otb::ogr::DataSource::Pointer source = otb::ogr::DataSource::New(
        this->m_source, otb::ogr::DataSource::Modes::Read);
    for (otb::ogr::DataSource::const_iterator lb=source->begin(), le=source->end(); lb != le; ++lb)
    {
        otb::ogr::Layer const& inputLayer = *lb;
        otb::ogr::Layer::const_iterator featIt = inputLayer.begin();
        for(; featIt!=inputLayer.end(); ++featIt)
        {
            OgrFeatureDescription ftDescr;
            ftDescr.featIt = featIt;
            fnc(ftDescr, payload);
        }
    }
    return true;
}

std::string PracticeShpReader::OgrFeatureDescription::GetFieldId() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_FieldIdFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetFieldSeqId() const
{
    return "-1";
}

std::string PracticeShpReader::OgrFeatureDescription::GetMainCrop() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_MainCropFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetCountryCode() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_CountryFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetYear() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_YearFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetVegetationStart() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_VegStartFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetHarvestStart() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_HarvestStartFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetHarvestEnd() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_HarvestEndFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetPractice() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_PracticeFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetPracticeType() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_PracticeTypeFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetPracticeStart() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_PracticeStartFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetPracticeEnd() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_PracticeEndFieldName.c_str()));
}

std::string PracticeShpReader::OgrFeatureDescription::GetS1Pix() const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsString(ogrFeat.GetFieldIndex(this->m_S1PixFieldName.c_str()));
}
