#include "TimeSeriesReader.h"

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
    // loop through the descriptors
    auto maccsMetadataReader = MACCSMetadataReaderType::New();
    auto spot4MetadataReader = SPOT4MetadataReaderType::New();
    for (auto it = begin; it != end; ++it) {
        const auto &desc = *it;

        ImageDescriptor id;
        if (auto meta = maccsMetadataReader->ReadMetadata(desc)) {
            // add the information to the list
            if (meta->Header.FixedHeader.Mission.find(LANDSAT) != std::string::npos) {
                // Interpret landsat product
                ProcessLandsat8Metadata(*meta, desc, id, td);
            } else if (meta->Header.FixedHeader.Mission.find(SENTINEL) != std::string::npos) {
                // Interpret sentinel product
                ProcessSentinel2Metadata(*meta, desc, id, td);
            } else {
                itkExceptionMacro("Unknown mission: " + meta->Header.FixedHeader.Mission);
            }

            m_Descriptors.push_back(id);

        }else if (auto meta = spot4MetadataReader->ReadMetadata(desc)) {
            // add the information to the list
            ProcessSpot4Metadata(*meta, desc, id, td);

            m_Descriptors.push_back(id);

        } else {
            itkExceptionMacro("Unable to read metadata from " << desc);
        }
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
    MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
    SPOT4MetadataReaderType::Pointer spot4MetadataReader = SPOT4MetadataReaderType::New();
    for (int i = startIndex; i < endIndex; i++) {
        const std::string& desc = descriptors[i];
        if (auto meta = maccsMetadataReader->ReadMetadata(desc)) {
            // check if the raster corresponds to the main mission
            if (meta->Header.FixedHeader.Mission.find(m_mission) != std::string::npos) {
                boost::filesystem::path rootFolder(desc);
                rootFolder.remove_filename();

                std::string imageFile = getMACCSRasterFileName(rootFolder, meta->ProductOrganization.ImageFiles, "_FRE");
                if (imageFile.size() == 0) {
                    imageFile = getMACCSRasterFileName(rootFolder, meta->ProductOrganization.ImageFiles, "_FRE_R1");
                }
                auto reader = getInt16ImageReader(imageFile);
                reader->GetOutput()->UpdateOutputInformation();
                float curRes = reader->GetOutput()->GetSpacing()[0];


                const float scale = (float)m_pixSize / curRes;
                td.m_imageWidth = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[0] / scale;
                td.m_imageHeight = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] / scale;

                auto origin = reader->GetOutput()->GetOrigin();
                auto spacing = reader->GetOutput()->GetSpacing();
                td.m_imageOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale - 1.0);
                td.m_imageOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale - 1.0);

                td.m_projection = reader->GetOutput()->GetProjectionRef();

                break;
            }

        }else if (auto meta = spot4MetadataReader->ReadMetadata(desc)) {
            // check if the raster corresponds to the main mission
            if (meta->Header.Ident.find(m_mission) != std::string::npos) {
                // get the root foloder from the descriptor file name
                boost::filesystem::path rootFolder(desc);
                rootFolder.remove_filename();

                const auto &imageFile = rootFolder / meta->Files.OrthoSurfCorrPente;
                auto reader = getInt16ImageReader(imageFile.string());
                reader->GetOutput()->UpdateOutputInformation();
                float curRes = reader->GetOutput()->GetSpacing()[0];


                const float scale = (float)m_pixSize / curRes;
                td.m_imageWidth = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[0] / scale;
                td.m_imageHeight = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] / scale;

                auto origin = reader->GetOutput()->GetOrigin();
                ImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
                td.m_imageOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale - 1.0);
                td.m_imageOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale - 1.0);

                td.m_projection = reader->GetOutput()->GetProjectionRef();

                break;
            }

        } else {
            itkExceptionMacro("Unable to read metadata from " << desc);
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

    m_SpotMaskFilters = SpotMaskFilterListType::New();
    m_SentinelMaskFilters = SentinelMaskFilterListType::New();

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

void TimeSeriesReader::ProcessSpot4Metadata(const SPOT4Metadata& meta, const std::string& filename, ImageDescriptor& descriptor, const TileData& td) {

    // Extract the raster date
    descriptor.aquisitionDate = formatSPOT4Date(meta.Header.DatePdv);

    // Save the descriptor fiel path
    descriptor.filename = filename;

    // save the mission
    descriptor.mission = SPOT;
    descriptor.isMain = m_mission.compare(descriptor.mission) == 0;

    boost::filesystem::path rootFolder(filename);
    rootFolder.remove_filename();

    // Get the spot bands
    getSpotBands(meta, rootFolder, td, descriptor);

    // The mask is built with the SpotMaskFilter
    descriptor.mask = getSpotMask(meta, rootFolder, td);
}

void TimeSeriesReader::ProcessLandsat8Metadata(const MACCSFileMetadata& meta, const std::string& filename, ImageDescriptor& descriptor, const TileData& td) {

    // Extract the raster date
    descriptor.aquisitionDate = meta.InstanceId.AcquisitionDate;

    // Save the descriptor fiel path
    descriptor.filename = filename;

    // save the mission
    descriptor.mission = LANDSAT;
    descriptor.isMain = m_mission.compare(descriptor.mission) == 0;

    boost::filesystem::path rootFolder(filename);
    rootFolder.remove_filename();

    // Get the bands
    getLandsatBands(meta, rootFolder, td, descriptor);

    // Get the mask
    descriptor.mask = getLandsatMask(meta, rootFolder,td);
}


void TimeSeriesReader::ProcessSentinel2Metadata(const MACCSFileMetadata& meta, const std::string& filename, ImageDescriptor& descriptor, const TileData& td) {

    // Extract the raster date
    descriptor.aquisitionDate = meta.InstanceId.AcquisitionDate;

    // Save the descriptor fiel path
    descriptor.filename = filename;

    // save the mission
    descriptor.mission = SENTINEL;
    descriptor.isMain = m_mission.compare(descriptor.mission) == 0;

    boost::filesystem::path rootFolder(filename);
    rootFolder.remove_filename();

    // Get the bands
    getSentinelBands(meta, rootFolder, td, descriptor);

    // Get the mask
    descriptor.mask = getSentinelMask(meta, rootFolder,td);
}

int TimeSeriesReader::getBandIndex(const std::vector<MACCSBand>& bands, const std::string& name) {
    for (const MACCSBand& band : bands) {
        if (band.Name == name) {
            return std::stoi(band.Id);
        }
    }
    return -1;
}

UInt8ImageReaderType::Pointer TimeSeriesReader::getUInt8ImageReader(const std::string& filePath) {
    auto reader = UInt8ImageReaderType::New();

    // set the file name
    reader->SetFileName(filePath + "?skipgeom=true");

    // add it to the list and return
    m_UInt8ImageReaderList->PushBack(reader);
    return reader;
}

Int16ImageReaderType::Pointer TimeSeriesReader::getInt16ImageReader(const std::string& filePath) {
    auto reader = Int16ImageReaderType::New();

    // set the file name
    reader->SetFileName(filePath + "?skipgeom=true");

    // add it to the list and return
    m_Int16ImageReaderList->PushBack(reader);
    return reader;
}

UInt16ImageReaderType::Pointer TimeSeriesReader::getUInt16ImageReader(const std::string& filePath) {
    auto reader = UInt16ImageReaderType::New();

    // set the file name
    reader->SetFileName(filePath + "?skipgeom=true");

    // add it to the list and return
    m_UInt16ImageReaderList->PushBack(reader);
    return reader;
}

UInt8VectorImageReaderType::Pointer TimeSeriesReader::getUInt8VectorImageReader(const std::string& filePath) {
    auto reader = UInt8VectorImageReaderType::New();

    // set the file name
    reader->SetFileName(filePath + "?skipgeom=true");

    // add it to the list and return
    m_UInt8VectorImageReaderList->PushBack(reader);
    return reader;
}

FloatVectorImageReaderType::Pointer TimeSeriesReader::getFloatVectorImageReader(const std::string& filePath) {
    auto reader = FloatVectorImageReaderType::New();

    // set the file name
    reader->SetFileName(filePath + "?skipgeom=true");

    // add it to the list and return
    m_Filters->PushBack(reader);
    return reader;
}

std::string TimeSeriesReader::getMACCSRasterFileName(const boost::filesystem::path &rootFolder, const std::vector<MACCSFileInformation>& imageFiles, const std::string& ending) {

    for (const MACCSFileInformation& fileInfo : imageFiles) {
        if (fileInfo.LogicalName.length() >= ending.length() &&
                0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return (rootFolder / (fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF")).string();
        }

    }
    return "";
}

std::string TimeSeriesReader::getMACCSMaskFileName(const boost::filesystem::path &rootFolder, const std::vector<MACCSAnnexInformation>& maskFiles, const std::string& ending) {

    for (const MACCSAnnexInformation& fileInfo : maskFiles) {
        if (fileInfo.File.LogicalName.length() >= ending.length() &&
                0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return (rootFolder / (fileInfo.File.FileLocation.substr(0, fileInfo.File.FileLocation.find_last_of('.')) + ".DBL.TIF")).string();
        }
    }
    return "";
}

void TimeSeriesReader::getSpotBands(const SPOT4Metadata& meta, const boost::filesystem::path &rootFolder, const TileData& td, ImageDescriptor &descriptor) {
    // Return the bands associated with a SPOT product
    // get the bands
    const auto &imageFile = rootFolder / meta.Files.OrthoSurfCorrPente;
    auto reader = getFloatVectorImageReader(imageFile.string());
    reader->GetOutput()->UpdateOutputInformation();

    auto resampledBands = getResampledBand<FloatVectorImageType>(reader->GetOutput(), td, false);

    auto channelExtractor1 = ExtractFloatChannelFilterType::New();
    channelExtractor1->SetInput(resampledBands);
    channelExtractor1->SetIndex(0);
    m_Filters->PushBack(channelExtractor1);

    auto channelExtractor2 = ExtractFloatChannelFilterType::New();
    channelExtractor2->SetInput(resampledBands);
    channelExtractor2->SetIndex(1);
    m_Filters->PushBack(channelExtractor2);

    auto channelExtractor3 = ExtractFloatChannelFilterType::New();
    channelExtractor3->SetInput(resampledBands);
    channelExtractor3->SetIndex(2);
    m_Filters->PushBack(channelExtractor3);

    auto channelExtractor4 = ExtractFloatChannelFilterType::New();
    channelExtractor4->SetInput(resampledBands);
    channelExtractor4->SetIndex(3);
    m_Filters->PushBack(channelExtractor4);

    // For SPOT series the bands are already in order. Only a resampling might be required
    descriptor.bands.push_back(channelExtractor1->GetOutput());
    descriptor.bands.push_back(channelExtractor2->GetOutput());
    descriptor.bands.push_back(channelExtractor3->GetOutput());
    descriptor.bands.push_back(channelExtractor4->GetOutput());
}

otb::Wrapper::UInt8ImageType::Pointer TimeSeriesReader::getSpotMask(const SPOT4Metadata& meta, const boost::filesystem::path &rootFolder, const TileData& td) {
    // Return the mask associated with a SPOT product
    // Get the validity mask
    const auto &maskFileDiv = rootFolder / meta.Files.MaskDiv;

    UInt8ImageReaderType::Pointer maskReaderDiv = getUInt8ImageReader(maskFileDiv.string());
    maskReaderDiv->GetOutput()->UpdateOutputInformation();

    // Get the saturation mask
    const auto &maskFileSat = rootFolder / meta.Files.MaskSaturation;
    UInt8ImageReaderType::Pointer maskReaderSat = getUInt8ImageReader(maskFileSat.string());

    // Get the clouds mask
    const auto &maskFileNua = rootFolder / meta.Files.MaskNua;
    UInt16ImageReaderType::Pointer maskReaderNua = getUInt16ImageReader(maskFileNua.string());

    // Build the SpotMaskFilter
    SpotMaskFilterType::Pointer spotMaskFilter = SpotMaskFilterType::New();
    spotMaskFilter->SetInputValidityMask(maskReaderDiv->GetOutput());
    spotMaskFilter->SetInputSaturationMask(maskReaderSat->GetOutput());
    spotMaskFilter->SetInputCloudsMask(maskReaderNua->GetOutput());

    // Add the filter to the list
    m_SpotMaskFilters->PushBack(spotMaskFilter);

    // Return the mask resampled to the required value
    return getResampledBand<UInt8ImageType>(spotMaskFilter->GetOutput(), td, true);
}

void TimeSeriesReader::getLandsatBands(const MACCSFileMetadata& meta, const boost::filesystem::path &rootFolder, const TileData& td, ImageDescriptor &descriptor) {
    // Return the bands associated with a LANDSAT product
    std::string imageFile = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE");
    auto reader = getFloatVectorImageReader(imageFile);
    reader->GetOutput()->UpdateOutputInformation();

    auto resampledBands = getResampledBand<FloatVectorImageType>(reader->GetOutput(), td, false);

    auto channelExtractor1 = ExtractFloatChannelFilterType::New();
    channelExtractor1->SetInput(resampledBands);
    channelExtractor1->SetIndex(2);
    m_Filters->PushBack(channelExtractor1);

    auto channelExtractor2 = ExtractFloatChannelFilterType::New();
    channelExtractor2->SetInput(resampledBands);
    channelExtractor2->SetIndex(3);
    m_Filters->PushBack(channelExtractor2);

    auto channelExtractor3 = ExtractFloatChannelFilterType::New();
    channelExtractor3->SetInput(resampledBands);
    channelExtractor3->SetIndex(4);
    m_Filters->PushBack(channelExtractor3);

    auto channelExtractor4 = ExtractFloatChannelFilterType::New();
    channelExtractor4->SetInput(resampledBands);
    channelExtractor4->SetIndex(5);
    m_Filters->PushBack(channelExtractor4);

    descriptor.bands.push_back(channelExtractor1->GetOutput());
    descriptor.bands.push_back(channelExtractor2->GetOutput());
    descriptor.bands.push_back(channelExtractor3->GetOutput());
    descriptor.bands.push_back(channelExtractor4->GetOutput());
}

otb::Wrapper::UInt8ImageType::Pointer TimeSeriesReader::getLandsatMask(const MACCSFileMetadata& meta, const boost::filesystem::path &rootFolder, const TileData& td) {
    // Return the mask associated with a LANDSAT product

    // Get the quality mask
    std::string maskFileQuality = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_QLT");

    auto maskReaderQuality = getUInt8VectorImageReader(maskFileQuality);
    maskReaderQuality->GetOutput()->UpdateOutputInformation();

    // Get the cloud mask
    std::string maskFileCloud = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_CLD");
    auto maskReaderCloud = getUInt8ImageReader(maskFileCloud);

    // Build the SentinelMaskFilter
    SentinelMaskFilterType::Pointer sentinelMaskFilter = SentinelMaskFilterType::New();
    sentinelMaskFilter->SetInputQualityMask(maskReaderQuality->GetOutput());
    sentinelMaskFilter->SetInputCloudsMask(maskReaderCloud->GetOutput());

    // Add the filter to the list
    m_SentinelMaskFilters->PushBack(sentinelMaskFilter);

    // Resample if needed
    return getResampledBand<UInt8ImageType>(sentinelMaskFilter->GetOutput(), td, true);
}

void TimeSeriesReader::getSentinelBands(const MACCSFileMetadata &meta,
                                        const boost::filesystem::path &rootFolder,
                                        const TileData &td,
                                        ImageDescriptor &descriptor)
{
    // Return the bands associated with a SENTINEL product
    // get the bands
    // Extract the first 3 bands form the first file.
    std::string imageFile1 =
            getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R1");
    auto reader1 = getInt16ImageReader(imageFile1);
    reader1->GetOutput()->UpdateOutputInformation();

    auto channelExtractor1 = ExtractChannelFilterType::New();
    channelExtractor1->SetInput(reader1->GetOutput());
    channelExtractor1->SetIndex(1);
    m_ChannelExtractors->PushBack(channelExtractor1);

    auto channelExtractor2 = ExtractChannelFilterType::New();
    channelExtractor2->SetInput(reader1->GetOutput());
    channelExtractor2->SetIndex(2);
    m_ChannelExtractors->PushBack(channelExtractor2);

    auto channelExtractor3 = ExtractChannelFilterType::New();
    channelExtractor3->SetInput(reader1->GetOutput());
    channelExtractor3->SetIndex(3);
    m_ChannelExtractors->PushBack(channelExtractor3);

    // Extract the last band form the second file.
    std::string imageFile2 =
            getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R2");
    auto reader2 = getInt16ImageReader(imageFile2);
    reader2->GetOutput()->UpdateOutputInformation();

    // Get the index of the SWIR band
    int swirIndex = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B11");
    auto channelExtractor4 = ExtractChannelFilterType::New();
    channelExtractor4->SetInput(reader2->GetOutput());
    channelExtractor4->SetIndex(swirIndex - 1);
    m_ChannelExtractors->PushBack(channelExtractor4);

    // Return the concatenation result
    auto b3Band = getResampledBand<FloatImageType>(channelExtractor1->GetOutput(), td, false);
    auto b4Band = getResampledBand<FloatImageType>(channelExtractor2->GetOutput(), td, false);
    auto b8Band = getResampledBand<FloatImageType>(channelExtractor3->GetOutput(), td, false);
    auto b11Band = getResampledBand<FloatImageType>(channelExtractor4->GetOutput(), td, false);

    descriptor.bands.push_back(b3Band);
    descriptor.bands.push_back(b4Band);
    descriptor.bands.push_back(b8Band);
    descriptor.bands.push_back(b11Band);

    getSentinelRedEdgeBands(meta, td, descriptor, reader1, reader2);
}

void TimeSeriesReader::getSentinelRedEdgeBands(const MACCSFileMetadata &,
                                               const TileData &,
                                               ImageDescriptor &,
                                               Int16ImageReaderType::Pointer,
                                               Int16ImageReaderType::Pointer)
{
}

otb::Wrapper::UInt8ImageType::Pointer TimeSeriesReader::getSentinelMask(const MACCSFileMetadata& meta, const boost::filesystem::path &rootFolder, const TileData& td) {
    // Return the mask associated with a SENTINEL product

    // Get the quality mask
    std::string maskFileQuality = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_QLT_R1");

    auto maskReaderQuality = getUInt8VectorImageReader(maskFileQuality);
    maskReaderQuality->GetOutput()->UpdateOutputInformation();

    // Get the cloud mask
    std::string maskFileCloud = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_CLD_R1");
    auto maskReaderCloud = getUInt8ImageReader(maskFileCloud);

    // Build the SentinelMaskFilter
    SentinelMaskFilterType::Pointer sentinelMaskFilter = SentinelMaskFilterType::New();
    sentinelMaskFilter->SetInputQualityMask(maskReaderQuality->GetOutput());
    sentinelMaskFilter->SetInputCloudsMask(maskReaderCloud->GetOutput());

    // Add the filter to the list
    m_SentinelMaskFilters->PushBack(sentinelMaskFilter);

    // Resample if needed
    return getResampledBand<UInt8ImageType>(sentinelMaskFilter->GetOutput(), td, true);
}
