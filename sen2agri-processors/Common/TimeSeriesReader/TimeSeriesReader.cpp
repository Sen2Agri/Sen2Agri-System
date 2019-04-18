#include "TimeSeriesReader.h"
#include "MetadataHelperFactory.h"

int getDaysFromEpoch(const std::string &date)
{
    struct tm tm = {};
    if (strptime(date.c_str(), "%Y%m%d", &tm) == NULL) {
        itkGenericExceptionMacro("Invalid value for a date: " + date);
    }
    int days = timegm(&tm) / 86400;
    return days;
}

void writeOutputDays(const std::vector<MissionDays> &days, const std::string &file)
{
    std::ofstream f(file);
    for (const auto &e : days) {
        for (auto d : e.days) {
            f << e.mission << ' ' << d << '\n';
        }
    }
}

std::vector<SensorPreferences> parseSensorPreferences(const std::vector<std::string> &sp)
{
    auto n = sp.size();

    std::vector<SensorPreferences> res;
    res.reserve(n / 2);

    for (size_t i = 0; i < n; i += 2) {
        const auto sensor = sp[i];
        const auto rateStr = sp[i + 1];
        auto rate = std::stoi(rateStr);
        res.emplace_back(SensorPreferences{sensor, static_cast<int>(i / 2), rate});
    }

    std::cout << "Sampling rates by sensor:\n";
    for (const auto &sensor : res) {
        std::cout << sensor.mission << ": " << sensor.samplingRate << '\n';
    }

    return res;
}

const std::vector<MissionDays> readOutputDays(const std::string &file)
{
    std::ifstream f(file);
    std::string last_mission;
    std::string mission;
    int day;
    std::vector<MissionDays> res;
    while (f >> mission >> day) {
        if (mission != last_mission) {
            res.emplace_back(MissionDays{mission, std::vector<int>()});
            last_mission = mission;
        }
        res.back().days.emplace_back(day);
    }
    return res;
}

void TimeSeriesReader::Build(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end, const TileData &td)
{
    auto factory = MetadataHelperFactory::New();

    for (auto it = begin; it != end; ++it) {
        const auto &desc = *it;

        m_ProductReaders.push_back(factory->GetMetadataHelper<float, uint8_t>(desc));
        const std::unique_ptr<MetadataHelper<float, uint8_t>> &pHelper = m_ProductReaders[m_ProductReaders.size()-1];
        ImageDescriptor id;
        ProcessMetadata(pHelper, desc, id, td);
        m_Descriptors.push_back(id);

    }

    // sort the descriptors after the aquisition date
    std::sort(m_Descriptors.begin(), m_Descriptors.end(), TimeSeriesReader::SortUnmergedMetadata);

    for (const ImageDescriptor& id : m_Descriptors) {
        for (const auto &b : id.bands) {
            m_FloatImageList->PushBack(b);
        }
        m_UInt8ImageList->PushBack(id.mask);
    }
    m_BandsConcat->SetInput(m_FloatImageList);
    m_MaskConcat->SetInput(m_UInt8ImageList);

    for (const ImageDescriptor& id : m_Descriptors) {
        if (id.mission == SENTINEL) {
            for (const auto &b : id.redEdgeBands) {
                m_RedEdgeBandImageList->PushBack(b);
            }
            m_RedEdgeMaskImageList->PushBack(id.mask);
        }
    }
    m_RedEdgeBandConcat->SetInput(m_RedEdgeBandImageList);
    m_RedEdgeMaskConcat->SetInput(m_RedEdgeMaskImageList);
}

void TimeSeriesReader::updateRequiredImageSize(const std::vector<std::string>& descriptors, int startIndex, int endIndex, TileData& td) {
    td.m_imageWidth = 0;
    td.m_imageHeight = 0;

    // look for the first raster belonging to the main mission
    auto factory = MetadataHelperFactory::New();
    for (int i = startIndex; i < endIndex; i++) {
        const std::string& desc = descriptors[i];
        std::unique_ptr<MetadataHelper<float, uint8_t>> pHelper = factory->GetMetadataHelper<float, uint8_t>(desc);
        if (pHelper->GetMissionShortName() == m_mission) {
            const std::vector<std::string> &firsResBandNames = pHelper->GetBandNamesForResolution(pHelper->GetProductResolutions()[0]);
            MetadataHelper<float, uint8_t>::VectorImageType::Pointer img = pHelper->GetImage(firsResBandNames);
            img->UpdateOutputInformation();
            float curRes = img->GetSpacing()[0];

            const float scale = (float)m_pixSize / curRes;
            td.m_imageWidth = img->GetLargestPossibleRegion().GetSize()[0] / scale;
            td.m_imageHeight = img->GetLargestPossibleRegion().GetSize()[1] / scale;

            auto origin = img->GetOrigin();
            ImageType::SpacingType spacing = img->GetSpacing();
            td.m_imageOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale - 1.0);
            td.m_imageOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale - 1.0);

            td.m_projection = img->GetProjectionRef();
            break;
        }
    }
}

void TimeSeriesReader::SetMission(std::string mission)
{
    m_mission = std::move(mission);
}

const std::string & TimeSeriesReader::GetMission() const
{
    return m_mission;
}

void TimeSeriesReader::SetPixelSize(float pixelSize)
{
    m_pixSize = pixelSize;
}

float TimeSeriesReader::GetPixelSize() const
{
    return m_pixSize;
}

void TimeSeriesReader::SetDescriptorList(std::vector<ImageDescriptor> descriptors)
{
    m_Descriptors = std::move(descriptors);
}

const std::vector<ImageDescriptor> & TimeSeriesReader::GetDescriptorList() const
{
    return m_Descriptors;
}

void TimeSeriesReader::SetSamplingRates(std::map<std::string, int> samplingRates)
{
    m_SamplingRates = std::move(samplingRates);
}

const std::map<std::string, int> & TimeSeriesReader::GetSamplingRates() const
{
    return m_SamplingRates;
}

TimeSeriesReader::TimeSeriesReader()
{
    m_ImageReaderList = ImageReaderListType::New();
    m_UInt8ImageReaderList = UInt8ImageReaderListType::New();
    m_Int16ImageReaderList = Int16ImageReaderListType::New();
    m_UInt16ImageReaderList = UInt16ImageReaderListType::New();
    m_UInt8VectorImageReaderList = UInt8VectorImageReaderListType::New();

    m_CastInt16FloatFilterFilst = CastInt16FloatFilterListType::New();

//    m_SpotMaskFilters = SpotMaskFilterListType::New();
//    m_SentinelMaskFilters = SentinelMaskFilterListType::New();

    m_ChannelExtractors = ExtractChannelListType::New();

    m_FloatImageList = FloatImageListType::New();
    m_UInt8ImageList = UInt8ImageListType::New();
    m_RedEdgeBandImageList = FloatImageListType::New();
    m_RedEdgeMaskImageList = UInt8ImageListType::New();

    m_BandsConcat = ConcatenateFloatImagesFilterType::New();
    m_MaskConcat = ConcatenateUInt8ImagesFilterType::New();
    m_RedEdgeBandConcat = ConcatenateFloatImagesFilterType::New();
    m_RedEdgeMaskConcat = ConcatenateUInt8ImagesFilterType::New();
}

TimeSeriesReader::~TimeSeriesReader()
{
}

bool TimeSeriesReader::SortUnmergedMetadata(const ImageDescriptor& o1, const ImageDescriptor& o2) {
    if (o1.isMain && !o2.isMain) {
        return true;
    }
    if (o2.isMain && !o1.isMain) {
        return false;
    }
    int missionComp = o1.mission.compare(o2.mission);
    if (missionComp == 0) {
        return o1.aquisitionDate.compare(o2.aquisitionDate) < 0;
    } else {
        return missionComp < 0;
    }
}

void TimeSeriesReader::ProcessMetadata(const std::unique_ptr<MetadataHelper<float, uint8_t>>& pHelper, const std::string& filename,
                                       ImageDescriptor& descriptor, const TileData& td) {
    // Extract the raster date
    descriptor.aquisitionDate = pHelper->GetAcquisitionDate();

    // Save the descriptor field path
    descriptor.filename = filename;

    // save the mission
    descriptor.mission = pHelper->GetMissionShortName();
    descriptor.isMain = m_mission.compare(descriptor.mission) == 0;

    // Get the product bands
    GetProductBands(pHelper, td, descriptor);

    // The mask is built from the product
    descriptor.mask = GetProductMask(pHelper, td);
}

void TimeSeriesReader::GetProductBands(const std::unique_ptr<MetadataHelper<float, uint8_t>>& pHelper, const TileData& td,
                                       ImageDescriptor &descriptor) {
    // Return extract Green, Red, Nir and Swir bands image
    const std::vector<std::string> &bandNames1 = {pHelper->GetGreenBandName(),
                                            pHelper->GetRedBandName(),
                                            pHelper->GetNirBandName()};
    const std::vector<std::string> &bandNames2 = {pHelper->GetSwirBandName()};

    std::vector<int> relBandIdxs1;
    std::vector<int> relBandIdxs2;
    MetadataHelper<float>::VectorImageType::Pointer img1 = pHelper->GetImage(bandNames1, &relBandIdxs1);
    MetadataHelper<float>::VectorImageType::Pointer img2 = pHelper->GetImage(bandNames2, &relBandIdxs2);
    img1->UpdateOutputInformation();
    img2->UpdateOutputInformation();

    auto resampledBands1 = getResampledBand<FloatVectorImageType>(img1, td, false);
    auto resampledBands2 = getResampledBand<FloatVectorImageType>(img2, td, false);

    for (int bandIndex: relBandIdxs1) {
        auto channelExtractor = ExtractFloatChannelFilterType::New();
        channelExtractor->SetInput(resampledBands1);
        channelExtractor->SetIndex(bandIndex);
        m_Filters->PushBack(channelExtractor);
        descriptor.bands.push_back(channelExtractor->GetOutput());
    }

    for (int bandIndex: relBandIdxs2) {
        auto channelExtractor = ExtractFloatChannelFilterType::New();
        channelExtractor->SetInput(resampledBands2);
        channelExtractor->SetIndex(bandIndex);
        m_Filters->PushBack(channelExtractor);
        descriptor.bands.push_back(channelExtractor->GetOutput());
    }

    getRedEdgeBands(pHelper, td, descriptor);
}

otb::Wrapper::UInt8ImageType::Pointer TimeSeriesReader::GetProductMask(const std::unique_ptr<MetadataHelper<float, uint8_t>>& pHelper,
                                                                       const TileData& td) {
    // Return the mask associated with the product with 1 if one of the flags is present and 0 otherwise
    MetadataHelper<float, uint8_t>::SingleBandMasksImageType::Pointer imgMsk = pHelper->GetMasksImage((MasksFlagType)(MSK_CLOUD|MSK_VALID|MSK_SAT), true);

    // Resample if needed
    return getResampledBand<UInt8ImageType>(imgMsk, td, true);
}

void TimeSeriesReader::getRedEdgeBands(const std::unique_ptr<MetadataHelper<float, uint8_t>>&,
                                               const TileData &,
                                               ImageDescriptor &)
{
}
