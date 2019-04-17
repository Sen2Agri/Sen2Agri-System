#include "GSAAShpAttributesTablesReader.h"
#include "otbOGRDataSourceWrapper.h"

bool GSAAShpAttributesTablesReader::ExtractAttributes(std::function<void (const AttributeEntry&)> fnc)
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
            fnc(ftDescr);
        }
    }
    return true;
}

int GSAAShpAttributesTablesReader::OgrFeatureDescription::GetFieldIndex(const char *pszName) const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldIndex(pszName);
}

 const char* GSAAShpAttributesTablesReader::OgrFeatureDescription::GetFieldAsString(int idx) const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
     return ogrFeat.GetFieldAsString(idx);
}

double GSAAShpAttributesTablesReader::OgrFeatureDescription::GetFieldAsDouble(int idx) const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsDouble(idx);
}
int GSAAShpAttributesTablesReader::OgrFeatureDescription::GetFieldAsInteger(int idx) const
{
    OGRFeature &ogrFeat = (*featIt).ogr();
    return ogrFeat.GetFieldAsInteger(idx);
}
