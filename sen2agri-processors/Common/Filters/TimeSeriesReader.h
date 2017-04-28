/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, softwareImageType
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/

#pragma once

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbWrapperApplicationRegistry.h"
#include "otbWrapperInputImageListParameter.h"
#include "otbWrapperTypes.h"

#include "otbVectorImage.h"
#include "otbImageList.h"
#include "otbImageListToVectorImageFilter.h"

#include "otbStreamingResampleImageFilter.h"
#include "otbGridResampleImageFilter.h"
#include "otbGenericRSResampleImageFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
//#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

// Filters
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "../Filters/otbSpotMaskFilter.h"
#include "../Filters/otbSentinelMaskFilter.h"

#include <string>
#include <map>

#include <boost/filesystem.hpp>

typedef otb::VectorImage<float, 2>                                 ImageType;
typedef otb::Wrapper::UInt8VectorImageType                         MaskType;

typedef otb::ImageList<otb::Wrapper::FloatImageType>               FloatImageListType;
typedef otb::ImageList<otb::Wrapper::UInt8ImageType>               UInt8ImageListType;

typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

typedef otb::ImageFileReader<otb::Wrapper::UInt8ImageType>         UInt8ImageReaderType;
typedef otb::ObjectList<UInt8ImageReaderType>                      UInt8ImageReaderListType;

typedef otb::ImageFileReader<otb::Wrapper::Int16VectorImageType>   Int16ImageReaderType;
typedef otb::ObjectList<Int16ImageReaderType>                      Int16ImageReaderListType;

typedef otb::ImageFileReader<otb::Wrapper::UInt16ImageType>        UInt16ImageReaderType;
typedef otb::ObjectList<UInt16ImageReaderType>                     UInt16ImageReaderListType;

typedef otb::ImageFileReader<otb::Wrapper::UInt8VectorImageType>   UInt8VectorImageReaderType;
typedef otb::ObjectList<UInt8VectorImageReaderType>                UInt8VectorImageReaderListType;

typedef otb::ImageFileReader<otb::Wrapper::FloatVectorImageType>   FloatVectorImageReaderType;

typedef otb::Image<float>                                          FloatImageType;
typedef otb::VectorImage<float>                                    FloatVectorImageType;
typedef otb::Image<uint8_t>                                        UInt8ImageType;
typedef otb::VectorImage<uint8_t>                                  UInt8VectorImageType;

typedef otb::GridResampleImageFilter<otb::Wrapper::UInt8VectorImageType, otb::Wrapper::UInt8VectorImageType, double>    UInt8VectorResampleFilterType;
typedef otb::GridResampleImageFilter<otb::Wrapper::Int16ImageType, otb::Wrapper::Int16ImageType, double>                Int16ResampleFilterType;
typedef otb::GridResampleImageFilter<otb::Wrapper::Int16ImageType, otb::Wrapper::FloatImageType, double>                Int16FloatResampleFilterType;

typedef otb::GenericRSResampleImageFilter<otb::Wrapper::FloatImageType, otb::Wrapper::FloatImageType>    FloatReprojectResampleFilterType;
typedef otb::GenericRSResampleImageFilter<otb::Wrapper::UInt8ImageType, otb::Wrapper::UInt8ImageType>    UInt8ReprojectResampleFilterType;
typedef otb::GenericRSResampleImageFilter<otb::Wrapper::UInt8VectorImageType, otb::Wrapper::UInt8VectorImageType>    UInt8VectorReprojectResampleFilterType;
typedef otb::GenericRSResampleImageFilter<otb::Wrapper::Int16ImageType, otb::Wrapper::Int16ImageType>                Int16ReprojectResampleFilterType;
typedef otb::GenericRSResampleImageFilter<otb::Wrapper::Int16ImageType, otb::Wrapper::FloatImageType>                Int16FloatReprojectResampleFilterType;

typedef otb::SpotMaskFilter                                 SpotMaskFilterType;
typedef otb::ObjectList<SpotMaskFilterType>                 SpotMaskFilterListType;

typedef otb::SentinelMaskFilter                             SentinelMaskFilterType;
typedef otb::ObjectList<SentinelMaskFilterType>             SentinelMaskFilterListType;

typedef itk::VectorIndexSelectionCastImageFilter<
            otb::Wrapper::Int16VectorImageType, otb::Wrapper::FloatImageType>        ExtractChannelFilterType;
typedef otb::ObjectList<ExtractChannelFilterType>                                    ExtractChannelListType;

typedef itk::VectorIndexSelectionCastImageFilter<
            otb::Wrapper::FloatVectorImageType, otb::Wrapper::FloatImageType>        ExtractFloatChannelFilterType;

typedef otb::ImageListToVectorImageFilter<otb::ImageList<otb::Wrapper::FloatImageType>, otb::Wrapper::FloatVectorImageType>       ConcatenateFloatImagesFilterType;
typedef otb::ObjectList<ConcatenateFloatImagesFilterType>                                                                         ConcatenateFloatImagesFilterListType;

typedef otb::ImageListToVectorImageFilter<otb::ImageList<otb::Wrapper::UInt8ImageType>, otb::Wrapper::UInt8VectorImageType>       ConcatenateUInt8ImagesFilterType;
typedef otb::ObjectList<ConcatenateUInt8ImagesFilterType>                                                                         ConcatenateUInt8ImagesFilterListType;

typedef otb::ImageListToVectorImageFilter<otb::ImageList<otb::Wrapper::Int16ImageType>, otb::Wrapper::Int16VectorImageType>       ConcatenateInt16ImagesFilterType;
typedef otb::ObjectList<ConcatenateInt16ImagesFilterType>                                                                         ConcatenateInt16ImagesFilterListType;

typedef itk::CastImageFilter<otb::Wrapper::Int16ImageType, otb::Wrapper::FloatImageType>    CastInt16FloatFilterType;
typedef otb::ObjectList<CastInt16FloatFilterType>                                           CastInt16FloatFilterListType;

typedef itk::MACCSMetadataReader                            MACCSMetadataReaderType;
typedef itk::SPOT4MetadataReader                            SPOT4MetadataReaderType;

#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"
#define SPOT       "SPOT"

#define MASK_TYPE_NUA   0
#define MASK_TYPE_SAT   1
#define MASK_TYPE_DIV   2

inline int getDaysFromEpoch(const std::string &date)
{
    struct tm tm = {};
    if (strptime(date.c_str(), "%Y%m%d", &tm) == NULL) {
        itkGenericExceptionMacro("Invalid value for a date: " + date);
    }
    return mktime(&tm) / 86400;
}

struct SourceImageMetadata {
    MACCSFileMetadata  msccsFileMetadata;
    SPOT4Metadata      spot4Metadata;
};

// Data related to the size and location of each tile
struct TileData {
    double                                m_imageWidth;
    double                                m_imageHeight;
    ImageType::PointType                  m_imageOrigin;
    std::string                           m_projection;
};

struct ImageDescriptor {
    // the descriptor file
    std::string filename;
    // the aquisition date in format YYYYMMDD
    std::string aquisitionDate;
    // the mission name
    std::string mission;
    // this product belongs to the main mission
    bool isMain;

    // The four required bands (Green, Red, NIR, SWIR) resampled to match the tile area
    std::vector<otb::Wrapper::FloatImageType::Pointer>                bands;
    // The combined mask
    otb::Wrapper::UInt8ImageType::Pointer          mask;

    // the format of the metadata
    unsigned char type;
    // the metadata
    SourceImageMetadata metadata;
};

//  Software Guide : EndCodeSnippet

class TimeSeriesReader : public itk::Object
{
public:
    typedef TimeSeriesReader Self;
    typedef itk::Object Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    itkNewMacro(Self)
    itkTypeMacro(TimeSeriesReader, itk::Object)

    itkSetMacro(IncludeRedEdge, bool)
    itkGetMacro(IncludeRedEdge, bool)

    void Build(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end, const TileData &td)
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
    }

    void updateRequiredImageSize(const std::vector<std::string>& descriptors, int startIndex, int endIndex, TileData& td) {
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

    void SetMission(std::string mission)
    {
        m_mission = std::move(mission);
    }

    const std::string & GetMission() const
    {
        return m_mission;
    }

    void SetPixelSize(float pixelSize)
    {
        m_pixSize = pixelSize;
    }

    float GetPixelSize() const
    {
        return m_pixSize;
    }

    void SetDescriptorList(std::vector<ImageDescriptor> descriptors)
    {
        m_Descriptors = std::move(descriptors);
    }

    const std::vector<ImageDescriptor> & GetDescriptorList() const
    {
        return m_Descriptors;
    }

    void SetSamplingRates(std::map<std::string, int> samplingRates)
    {
        m_SamplingRates = std::move(samplingRates);
    }

    const std::map<std::string, int> & GetSamplingRates() const
    {
        return m_SamplingRates;
    }

//    void SetSensorOutDays(std::map<std::string, std::vector<int> > sensorOutDays)
//    {
//        m_SensorOutDays = std::move(sensorOutDays);
//    }

//    const std::map<std::string, std::vector<int> > & GetSensorOutDays() const
//    {
//        return m_SensorOutDays;
//    }

protected:
    std::string m_mission;
    float m_pixSize;
    std::vector<ImageDescriptor> m_Descriptors;
    std::map<std::string, int> m_SamplingRates;
    std::map<std::string, std::vector<int> > m_SensorOutDays;
    bool m_IncludeRedEdge;

    ImageReaderListType::Pointer             m_ImageReaderList;
    UInt8ImageReaderListType::Pointer        m_UInt8ImageReaderList;
    Int16ImageReaderListType::Pointer        m_Int16ImageReaderList;
    UInt16ImageReaderListType::Pointer       m_UInt16ImageReaderList;
    UInt8VectorImageReaderListType::Pointer  m_UInt8VectorImageReaderList;

    CastInt16FloatFilterListType::Pointer        m_CastInt16FloatFilterFilst;

    SpotMaskFilterListType::Pointer                   m_SpotMaskFilters;
    SentinelMaskFilterListType::Pointer               m_SentinelMaskFilters;
    ExtractChannelListType::Pointer                   m_ChannelExtractors;

    FloatImageListType::Pointer                       m_FloatImageList;
    UInt8ImageListType::Pointer                       m_UInt8ImageList;
    ConcatenateFloatImagesFilterType::Pointer         m_BandsConcat;
    ConcatenateUInt8ImagesFilterType::Pointer         m_MaskConcat;

    TimeSeriesReader()
        : m_IncludeRedEdge()
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
        m_BandsConcat = ConcatenateFloatImagesFilterType::New();
        m_MaskConcat = ConcatenateUInt8ImagesFilterType::New();
    }

    virtual ~TimeSeriesReader()
    {
    }

    // Sort the descriptors based on the aquisition date
    static bool SortUnmergedMetadata(const ImageDescriptor& o1, const ImageDescriptor& o2) {
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

    // Process a SPOT4 metadata structure and extract the needed bands and masks.
    void ProcessSpot4Metadata(const SPOT4Metadata& meta, const std::string& filename, ImageDescriptor& descriptor, const TileData& td) {

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

    // Process a LANDSAT8 metadata structure and extract the needed bands and masks.
    void ProcessLandsat8Metadata(const MACCSFileMetadata& meta, const std::string& filename, ImageDescriptor& descriptor, const TileData& td) {

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


    // Process a SENTINEL2 metadata structure and extract the needed bands and masks.
    void ProcessSentinel2Metadata(const MACCSFileMetadata& meta, const std::string& filename, ImageDescriptor& descriptor, const TileData& td) {

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

    // Get the id of the band. Return -1 if band not found.
    int getBandIndex(const std::vector<MACCSBand>& bands, const std::string& name) {
        for (const MACCSBand& band : bands) {
            if (band.Name == name) {
                return std::stoi(band.Id);
            }
        }
        return -1;
    }

    UInt8ImageReaderType::Pointer getUInt8ImageReader(const std::string& filePath) {
        auto reader = UInt8ImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);

        // add it to the list and return
        m_UInt8ImageReaderList->PushBack(reader);
        return reader;
    }

    Int16ImageReaderType::Pointer getInt16ImageReader(const std::string& filePath) {
        auto reader = Int16ImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);

        // add it to the list and return
        m_Int16ImageReaderList->PushBack(reader);
        return reader;
    }

    UInt16ImageReaderType::Pointer getUInt16ImageReader(const std::string& filePath) {
        auto reader = UInt16ImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);

        // add it to the list and return
        m_UInt16ImageReaderList->PushBack(reader);
        return reader;
    }

    UInt8VectorImageReaderType::Pointer getUInt8VectorImageReader(const std::string& filePath) {
        auto reader = UInt8VectorImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);

        // add it to the list and return
        m_UInt8VectorImageReaderList->PushBack(reader);
        return reader;
    }

    FloatVectorImageReaderType::Pointer getFloatVectorImageReader(const std::string& filePath) {
        auto reader = FloatVectorImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);

        // add it to the list and return
        m_Filters->PushBack(reader);
        return reader;
    }

    // Return the path to a file for which the name end in the ending
    std::string getMACCSRasterFileName(const boost::filesystem::path &rootFolder, const std::vector<MACCSFileInformation>& imageFiles, const std::string& ending) {

        for (const MACCSFileInformation& fileInfo : imageFiles) {
            if (fileInfo.LogicalName.length() >= ending.length() &&
                    0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
                return (rootFolder / (fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF")).string();
            }

        }
        return "";
    }


    // Return the path to a file for which the name end in the ending
    std::string getMACCSMaskFileName(const boost::filesystem::path &rootFolder, const std::vector<MACCSAnnexInformation>& maskFiles, const std::string& ending) {

        for (const MACCSAnnexInformation& fileInfo : maskFiles) {
            if (fileInfo.File.LogicalName.length() >= ending.length() &&
                    0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
                return (rootFolder / (fileInfo.File.FileLocation.substr(0, fileInfo.File.FileLocation.find_last_of('.')) + ".DBL.TIF")).string();
            }
        }
        return "";
    }

    void getSpotBands(const SPOT4Metadata& meta, const boost::filesystem::path &rootFolder, const TileData& td, ImageDescriptor &descriptor) {
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

    otb::Wrapper::UInt8ImageType::Pointer getSpotMask(const SPOT4Metadata& meta, const boost::filesystem::path &rootFolder, const TileData& td) {
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

    void getLandsatBands(const MACCSFileMetadata& meta, const boost::filesystem::path &rootFolder, const TileData& td, ImageDescriptor &descriptor) {
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

    otb::Wrapper::UInt8ImageType::Pointer getLandsatMask(const MACCSFileMetadata& meta, const boost::filesystem::path &rootFolder, const TileData& td) {
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

    void getSentinelBands(const MACCSFileMetadata& meta, const boost::filesystem::path &rootFolder, const TileData& td, ImageDescriptor &descriptor) {
        // Return the bands associated with a SENTINEL product
        // get the bands
        //Extract the first 3 bands form the first file.
        std::string imageFile1 = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R1");
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

        //Extract the last band form the second file.
        std::string imageFile2 = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R2");
        auto reader2 = getInt16ImageReader(imageFile2);
        reader2->GetOutput()->UpdateOutputInformation();

        // Get the index of the SWIR band
        int swirIndex = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B11");
        auto channelExtractor4 = ExtractChannelFilterType::New();
        channelExtractor4->SetInput(reader2->GetOutput());
        channelExtractor4->SetIndex(swirIndex - 1);
        m_ChannelExtractors->PushBack(channelExtractor4);

        // Return the concatenation result
        descriptor.bands.push_back(getResampledBand<FloatImageType>(channelExtractor1->GetOutput(), td, false));
        descriptor.bands.push_back(getResampledBand<FloatImageType>(channelExtractor2->GetOutput(), td, false));
        descriptor.bands.push_back(getResampledBand<FloatImageType>(channelExtractor3->GetOutput(), td, false));
        descriptor.bands.push_back(getResampledBand<FloatImageType>(channelExtractor4->GetOutput(), td, false));

        if (m_IncludeRedEdge) {
            ExtractChannelFilterType::Pointer channelExtractor;

            int redEdge1Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B5");
            channelExtractor = ExtractChannelFilterType::New();
            channelExtractor->SetInput(reader2->GetOutput());
            channelExtractor->SetIndex(redEdge1Index - 1);
            m_Filters->PushBack(channelExtractor);
            descriptor.bands.push_back(getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false));

            int redEdge2Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B6");
            channelExtractor = ExtractChannelFilterType::New();
            channelExtractor->SetInput(reader2->GetOutput());
            channelExtractor->SetIndex(redEdge2Index - 1);
            m_Filters->PushBack(channelExtractor);
            descriptor.bands.push_back(getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false));

            int redEdge3Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B7");
            channelExtractor = ExtractChannelFilterType::New();
            channelExtractor->SetInput(reader2->GetOutput());
            channelExtractor->SetIndex(redEdge3Index - 1);
            m_Filters->PushBack(channelExtractor);
            descriptor.bands.push_back(getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false));

            int redEdge4Index = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B8A");
            channelExtractor = ExtractChannelFilterType::New();
            channelExtractor->SetInput(reader2->GetOutput());
            channelExtractor->SetIndex(redEdge4Index - 1);
            m_Filters->PushBack(channelExtractor);
            descriptor.bands.push_back(getResampledBand<FloatImageType>(channelExtractor->GetOutput(), td, false));
        }
    }

    otb::Wrapper::UInt8ImageType::Pointer getSentinelMask(const MACCSFileMetadata& meta, const boost::filesystem::path &rootFolder, const TileData& td) {
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

    otb::ObjectList<itk::ProcessObject>::Pointer m_Filters = otb::ObjectList<itk::ProcessObject>::New();

    template<typename ImageType>
    typename ImageType::Pointer getResampledBand(const typename ImageType::Pointer& image, const TileData& td, bool isMask)
    {
        image->UpdateOutputInformation();
        auto imageSize = image->GetLargestPossibleRegion().GetSize();


        // Evaluate size
        typename ImageType::SizeType recomputedSize;
        recomputedSize[0] = td.m_imageWidth;
        recomputedSize[1] = td.m_imageHeight;

        std::string inputProjection = image->GetProjectionRef();
        bool sameProjection = inputProjection == td.m_projection;
        bool sameOrigin = image->GetOrigin() == td.m_imageOrigin;
        if (imageSize == recomputedSize && sameOrigin && sameProjection)
        {
            return image;
        }

        // Evaluate spacing
        typename ImageType::SpacingType outputSpacing;
        outputSpacing[0] = m_pixSize;
        outputSpacing[1] = -m_pixSize;


        typedef typename itk::InterpolateImageFunction<ImageType, double>     InterpolateImageFunctionType;

        typename InterpolateImageFunctionType::Pointer interpolator;

        // Set the interpolator
        typedef itk::NearestNeighborInterpolateImageFunction<ImageType, double>             NearestNeighborInterpolationType;
        typedef otb::BCOInterpolateImageFunction<ImageType, double>                         BicubicInterpolationType;

        float edgeValue;
        if (isMask)
        {
            interpolator = NearestNeighborInterpolationType::New();
            edgeValue = 1;
        }
        else
        {
            interpolator = BicubicInterpolationType::New();
            edgeValue = -10000;
        }

        typedef typename ImageType::PixelType PixelType;

        PixelType edgePixel;
        itk::NumericTraits<PixelType>::SetLength(edgePixel, image->GetNumberOfComponentsPerPixel());
        edgePixel = edgeValue * itk::NumericTraits<PixelType>::OneValue(edgePixel);

        typename ImageType::Pointer output;
        if (sameProjection)
        {
            if (sameOrigin)
            {
                typedef otb::GridResampleImageFilter<ImageType, ImageType, double>  ResampleFilterType;

                typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
                m_Filters->PushBack(resampler);

                resampler->SetInput(image);
                resampler->SetInterpolator(interpolator);
                resampler->SetOutputParametersFromImage(image);
                resampler->SetOutputSpacing(outputSpacing);
                resampler->SetOutputOrigin(td.m_imageOrigin);
                resampler->SetOutputSize(recomputedSize);
                resampler->SetEdgePaddingValue(edgePixel);
                resampler->SetCheckOutputBounds(false);

                output = resampler->GetOutput();
            }
            else
            {
                typedef otb::StreamingResampleImageFilter<ImageType, ImageType, double>  ResampleFilterType;

                typename ResampleFilterType::Pointer resampler = ResampleFilterType::New();
                m_Filters->PushBack(resampler);

                resampler->SetInput(image);
                resampler->SetInterpolator(interpolator);
                resampler->SetOutputParametersFromImage(image);
                resampler->SetOutputSpacing(outputSpacing);
                resampler->SetOutputOrigin(td.m_imageOrigin);
                resampler->SetOutputSize(recomputedSize);
                resampler->SetEdgePaddingValue(edgePixel);

                output = resampler->GetOutput();
            }
        }
        else
        {
            typedef otb::GenericRSResampleImageFilter<ImageType, ImageType>     ReprojectResampleFilterType;

            typename ReprojectResampleFilterType::Pointer resampler = ReprojectResampleFilterType::New();
            m_Filters->PushBack(resampler);

            resampler->SetInput(image);
            resampler->SetInputProjectionRef(inputProjection);
            resampler->SetOutputProjectionRef(td.m_projection);
            resampler->SetInputKeywordList(image->GetImageKeywordlist());
            resampler->SetInterpolator(interpolator);
            resampler->SetOutputSpacing(outputSpacing);
            resampler->SetDisplacementFieldSpacing(outputSpacing * 20);
            resampler->SetOutputOrigin(td.m_imageOrigin);
            resampler->SetOutputSize(recomputedSize);
            resampler->SetEdgePaddingValue(edgePixel);

            output = resampler->GetOutput();
        }
        return output;
    }

    // build the date for spot products as YYYYMMDD
    inline std::string formatSPOT4Date(const std::string& date) {
        return date.substr(0,4) + date.substr(5,2) + date.substr(8,2);
    }

    int getBandCount(const std::string &sensor) {
        if (sensor == "SENTINEL") {
            if (GetIncludeRedEdge()) {
                return 8;
            } else {
                return 4;
            }
        } else if (sensor == "SPOT" || sensor == "LANDSAT") {
            return 4;
        }
        itkGenericExceptionMacro("Unknown sensor " << sensor);
    }
};

struct MissionDays
{
    std::string mission;
    std::vector<int> days;
};

struct SensorPreferences
{
    std::string mission;
    int priority;
    int samplingRate;
};

template<typename TTimeSeriesReaderList>
std::vector<MissionDays> getOutputDays(TTimeSeriesReaderList preprocessors, bool resample, const std::vector<SensorPreferences> &sp)
{
    std::map<std::string, SensorPreferences> spMap;
    for (const auto &e : sp) {
        spMap[e.mission] = e;
    }

    std::map<std::string, std::set<int> >    sensorInDays;
    std::map<std::string, std::vector<int> > sensorOutDays;

    for (unsigned int i = 0; i < preprocessors->Size(); i++) {
        for (const auto &id : preprocessors->GetNthElement(i)->GetDescriptorList()) {
            auto inDay = getDaysFromEpoch(id.aquisitionDate);
            sensorInDays[id.mission].emplace(inDay);
        }
    }

    // loop through the sensors to determinte the output dates
    for (const auto& sensor : sensorInDays) {
        std::vector<int> outDates;
        if (resample) {
            auto it = spMap.find(sensor.first);
            if (it == spMap.end()) {
                itkGenericExceptionMacro("Sampling rate required for sensor " << sensor.first);
            }
            auto rate = it->second.samplingRate;

            auto last = *sensor.second.rbegin();
            for (int date = *sensor.second.begin(); date <= last; date += rate) {
                outDates.emplace_back(date);
            }
        } else {
            outDates.insert(outDates.end(), sensor.second.begin(), sensor.second.end());
        }
        sensorOutDays[sensor.first] = outDates;
    }

    std::vector<MissionDays> res;
    for (const auto &e : sensorOutDays) {
        res.emplace_back(MissionDays{std::move(e.first), std::move(e.second)});
    }
    std::sort(std::begin(res), std::end(res), [&](const MissionDays &e1, const MissionDays &e2) {
        return spMap[e1.mission].priority < spMap[e2.mission].priority;
    });
    return res;
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
