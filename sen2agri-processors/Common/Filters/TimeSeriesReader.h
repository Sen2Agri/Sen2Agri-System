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

#pragma once

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbWrapperApplicationRegistry.h"
#include "otbWrapperInputImageListParameter.h"

#include "otbVectorImage.h"
#include "otbImageList.h"

#include "otbStreamingResampleImageFilter.h"
#include "otbGridResampleImageFilter.h"

//Transform
#include "itkScalableAffineTransform.h"
//#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"
#include "../../MACCSMetadata/include/SPOT4MetadataReader.hpp"

// Filters
#include "otbMultiChannelExtractROI.h"
#include "otbConcatenateVectorImagesFilter.h"
#include "../Filters/otbCropTypeFeatureExtractionFilter.h"
#include "../Filters/otbTemporalResamplingFilter.h"
#include "../Filters/otbTemporalMergingFilter.h"

#include "../Filters/otbSpotMaskFilter.h"
#include "../Filters/otbLandsatMaskFilter.h"
#include "../Filters/otbSentinelMaskFilter.h"

#include <string>

typedef otb::VectorImage<float, 2>                                 ImageType;

typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

//typedef otb::StreamingResampleImageFilter<ImageType, ImageType, double>    ResampleFilterType;
typedef otb::GridResampleImageFilter<ImageType, ImageType, double>    ResampleFilterType;
typedef otb::ObjectList<ResampleFilterType>                           ResampleFilterListType;
typedef otb::BCOInterpolateImageFunction<ImageType,
                                            double>          BicubicInterpolationType;
typedef itk::LinearInterpolateImageFunction<ImageType,
                                            double>          LinearInterpolationType;
typedef itk::NearestNeighborInterpolateImageFunction<ImageType, double>             NearestNeighborInterpolationType;

//typedef itk::IdentityTransform<double, ImageType::ImageDimension>      IdentityTransformType;
typedef itk::ScalableAffineTransform<double, ImageType::ImageDimension> ScalableTransformType;
typedef ScalableTransformType::OutputVectorType                         OutputVectorType;

typedef otb::SpotMaskFilter<ImageType>                      SpotMaskFilterType;
typedef otb::ObjectList<SpotMaskFilterType>                 SpotMaskFilterListType;

typedef otb::LandsatMaskFilter<ImageType>                   LandsatMaskFilterType;
typedef otb::ObjectList<LandsatMaskFilterType>              LandsatMaskFilterListType;

typedef otb::SentinelMaskFilter<ImageType>                  SentinelMaskFilterType;
typedef otb::ObjectList<SentinelMaskFilterType>             SentinelMaskFilterListType;

typedef otb::MultiChannelExtractROI<float, float>
                                                            MultiChannelExtractROIType;
typedef otb::ObjectList<MultiChannelExtractROIType>         MultiChannelExtractROIListType;

typedef otb::ConcatenateVectorImagesFilter<ImageType>       ConcatenateVectorImagesFilterType;
typedef otb::ObjectList<ConcatenateVectorImagesFilterType>  ConcatenateVectorImagesFilterListType;

typedef otb::TemporalResamplingFilter<ImageType>            TemporalResamplingFilterType;
typedef otb::ObjectList<TemporalResamplingFilterType>       TemporalResamplingFilterListType;

typedef otb::CropTypeFeatureExtractionFilter<ImageType>     CropTypeFeatureExtractionFilterType;
typedef otb::ObjectList<CropTypeFeatureExtractionFilterType>
                                                            CropTypeFeatureExtractionFilterListType;

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
    ImageType::Pointer          bands;
    // The combined mask
    ImageType::Pointer          mask;

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
    }


    // TODO we're opening the files and reading metadata twice, here and in Build()
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
                    std::string rootFolder = extractFolder(desc);
                    std::string imageFile = getMACCSRasterFileName(rootFolder, meta->ProductOrganization.ImageFiles, "_FRE");
                    if (imageFile.size() == 0) {
                        imageFile = getMACCSRasterFileName(rootFolder, meta->ProductOrganization.ImageFiles, "_FRE_R1");
                    }
                    ImageReaderType::Pointer reader = getReader(imageFile);
                    reader->UpdateOutputInformation();
                    float curRes = reader->GetOutput()->GetSpacing()[0];


                    const float scale = (float)m_pixSize / curRes;
                    td.m_imageWidth = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[0] / scale;
                    td.m_imageHeight = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] / scale;

                    auto origin = reader->GetOutput()->GetOrigin();
                    auto spacing = reader->GetOutput()->GetSpacing();
                    td.m_imageOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale - 1.0);
                    td.m_imageOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale - 1.0);

                    break;
                }

            }else if (auto meta = spot4MetadataReader->ReadMetadata(desc)) {
                // check if the raster corresponds to the main mission
                if (meta->Header.Ident.find(m_mission) != std::string::npos) {
                    // get the root foloder from the descriptor file name
                    std::string rootFolder = extractFolder(desc);
                    std::string imageFile = rootFolder + meta->Files.OrthoSurfCorrPente;
                    auto reader = getReader(imageFile);
                    reader->UpdateOutputInformation();
                    float curRes = reader->GetOutput()->GetSpacing()[0];


                    const float scale = (float)m_pixSize / curRes;
                    td.m_imageWidth = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[0] / scale;
                    td.m_imageHeight = reader->GetOutput()->GetLargestPossibleRegion().GetSize()[1] / scale;

                    auto origin = reader->GetOutput()->GetOrigin();
                    ImageType::SpacingType spacing = reader->GetOutput()->GetSpacing();
                    td.m_imageOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale - 1.0);
                    td.m_imageOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale - 1.0);

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

    void SetSensorOutDays(std::map<std::string, std::vector<int> > sensorOutDays)
    {
        m_SensorOutDays = std::move(sensorOutDays);
    }

    const std::map<std::string, std::vector<int> > & GetSensorOutDays() const
    {
        return m_SensorOutDays;
    }

protected:
    std::string m_mission;
    float m_pixSize;
    std::vector<ImageDescriptor> m_Descriptors;
    std::map<std::string, int> m_SamplingRates;
    std::map<std::string, std::vector<int> > m_SensorOutDays;

    ImageReaderListType::Pointer          m_ImageReaderList;
    ResampleFilterListType::Pointer       m_ResamplersList;

    SpotMaskFilterListType::Pointer                   m_SpotMaskFilters;
    LandsatMaskFilterListType::Pointer                m_LandsatMaskFilters;
    SentinelMaskFilterListType::Pointer               m_SentinelMaskFilters;
    MultiChannelExtractROIListType::Pointer           m_ChannelExtractors;

    ConcatenateVectorImagesFilterListType::Pointer    m_ImageMergers;

    TemporalResamplingFilterType::Pointer             m_TemporalResampler;
    CropTypeFeatureExtractionFilterType::Pointer      m_FeatureExtractor;


    TimeSeriesReader()
    {
        m_ResamplersList = ResampleFilterListType::New();
        m_ImageReaderList = ImageReaderListType::New();


        m_SpotMaskFilters = SpotMaskFilterListType::New();
        m_LandsatMaskFilters = LandsatMaskFilterListType::New();
        m_SentinelMaskFilters = SentinelMaskFilterListType::New();

        m_ChannelExtractors = MultiChannelExtractROIListType::New();

        m_ImageMergers = ConcatenateVectorImagesFilterListType::New();

        m_TemporalResampler = TemporalResamplingFilterType::New();
        m_FeatureExtractor = CropTypeFeatureExtractionFilterType::New();
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

        // get the root foloder from the descriptor file name
        std::string rootFolder = extractFolder(filename);

        // Get the spot bands
        descriptor.bands = getSpotBands(meta, rootFolder, td);

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

        // get the root foloder from the descriptor file name
        std::string rootFolder = extractFolder(filename);

        // Get the bands
        descriptor.bands = getLandsatBands(meta, rootFolder,td);

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

        // get the root foloder from the descriptor file name
        std::string rootFolder = extractFolder(filename);

        // Get the bands
        descriptor.bands = getSentinelBands(meta, rootFolder,td);

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

    // get a reader from the file path
    ImageReaderType::Pointer getReader(const std::string& filePath) {
        ImageReaderType::Pointer reader = ImageReaderType::New();

        // set the file name
        reader->SetFileName(filePath);

        // add it to the list and return
        m_ImageReaderList->PushBack(reader);
        return reader;
    }

    // Return the path to a file for which the name end in the ending
    std::string getMACCSRasterFileName(const std::string& rootFolder, const std::vector<MACCSFileInformation>& imageFiles, const std::string& ending) {

        for (const MACCSFileInformation& fileInfo : imageFiles) {
            if (fileInfo.LogicalName.length() >= ending.length() &&
                    0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
                return rootFolder + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + ".DBL.TIF";
            }

        }
        return "";
    }


    // Return the path to a file for which the name end in the ending
    std::string getMACCSMaskFileName(const std::string& rootFolder, const std::vector<MACCSAnnexInformation>& maskFiles, const std::string& ending) {

        for (const MACCSAnnexInformation& fileInfo : maskFiles) {
            if (fileInfo.File.LogicalName.length() >= ending.length() &&
                    0 == fileInfo.File.LogicalName.compare (fileInfo.File.LogicalName.length() - ending.length(), ending.length(), ending)) {
                return rootFolder + fileInfo.File.FileLocation.substr(0, fileInfo.File.FileLocation.find_last_of('.')) + ".DBL.TIF";
            }
        }
        return "";
    }

    // TODO replace with Boost.Filesystem
    // Extract the folder from a given path.
    std::string extractFolder(const std::string& filename) {
        size_t pos = filename.find_last_of("/\\");
        if (pos == std::string::npos) {
            return "";
        }

        return filename.substr(0, pos) + "/";
    }

    // Get the path to the Spot4 raster
    std::string getSPOT4RasterFileName(const SPOT4Metadata & desc, const std::string& folder) {
        return folder + desc.Files.OrthoSurfCorrPente;
    }

    // Return the path to a SPOT4 mask file
    std::string getSPOT4MaskFileName(const SPOT4Metadata & desc, const std::string& rootFolder, const unsigned char maskType) {

        std::string file;
        switch (maskType) {
        case MASK_TYPE_NUA:
            file = desc.Files.MaskNua;
            break;
        case MASK_TYPE_DIV:
            file = desc.Files.MaskDiv;
            break;
        case MASK_TYPE_SAT:
        default:
            file = desc.Files.MaskSaturation;
            break;
        }

        return rootFolder + file;
    }

    ImageType::Pointer getSpotBands(const SPOT4Metadata& meta, const std::string& rootFolder, const TileData& td) {
        // Return the bands associated with a SPOT product
        // get the bands
        std::string imageFile = rootFolder + meta.Files.OrthoSurfCorrPente;
        ImageReaderType::Pointer reader = getReader(imageFile);
        reader->UpdateOutputInformation();
        float curRes = reader->GetOutput()->GetSpacing()[0];

        return reader->GetOutput();

        // For SPOT series the bands are already in order. Only a resampling might be required
        return getResampledBands(reader->GetOutput(), td, curRes, false);
    }

    ImageType::Pointer getSpotMask(const SPOT4Metadata& meta, const std::string& rootFolder, const TileData& td) {
        // Return the mask associated with a SPOT product
        // Get the validity mask
        std::string maskFileDiv = rootFolder + meta.Files.MaskDiv;
        ImageReaderType::Pointer maskReaderDiv = getReader(maskFileDiv);
        maskReaderDiv->UpdateOutputInformation();
        float curRes = maskReaderDiv->GetOutput()->GetSpacing()[0];

        // Get the saturation mask
        std::string maskFileSat = rootFolder + meta.Files.MaskSaturation;
        ImageReaderType::Pointer maskReaderSat = getReader(maskFileSat);

        // Get the clouds mask
        std::string maskFileNua = rootFolder + meta.Files.MaskNua;
        ImageReaderType::Pointer maskReaderNua = getReader(maskFileNua);

        // Build the SpotMaskFilter
        SpotMaskFilterType::Pointer spotMaskFilter = SpotMaskFilterType::New();
        spotMaskFilter->SetInputValidityMask(maskReaderDiv->GetOutput());
        spotMaskFilter->SetInputSaturationMask(maskReaderSat->GetOutput());
        spotMaskFilter->SetInputCloudsMask(maskReaderNua->GetOutput());

        // Add the filter to the list
        m_SpotMaskFilters->PushBack(spotMaskFilter);

        // Return the mask resampled to the required value
        return getResampledBands(spotMaskFilter->GetOutput(), td, curRes, true);
    }

    ImageType::Pointer getLandsatBands(const MACCSFileMetadata& meta, const std::string& rootFolder, const TileData& td) {
        // Return the bands associated with a LANDSAT product
        // get the bands
        std::string imageFile = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE");
        ImageReaderType::Pointer reader = getReader(imageFile);
        reader->UpdateOutputInformation();
        float curRes = reader->GetOutput()->GetSpacing()[0];

        // Extract the bands from 3 to 6
        MultiChannelExtractROIType::Pointer chExtractor = MultiChannelExtractROIType::New();
        chExtractor->SetInput(reader->GetOutput());
        chExtractor->SetFirstChannel(3);
        chExtractor->SetLastChannel(6);
        m_ChannelExtractors->PushBack(chExtractor);

        // A resampling might be required
        return getResampledBands(chExtractor->GetOutput(), td, curRes, false);
    }

    ImageType::Pointer getLandsatMask(const MACCSFileMetadata& meta, const std::string& rootFolder, const TileData& td) {
        // Return the mask associated with a LANDSAT product
        // Get the quality mask
        std::string maskFileQuality = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_QLT");
        ImageReaderType::Pointer maskReaderQuality = getReader(maskFileQuality);
        maskReaderQuality->UpdateOutputInformation();
        float curRes = maskReaderQuality->GetOutput()->GetSpacing()[0];

         // Get the cloud mask
        std::string maskFileCloud = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_CLD");
        ImageReaderType::Pointer maskReaderCloud = getReader(maskFileCloud);

        // Build the LandsatMaskFilter
        LandsatMaskFilterType::Pointer landsatMaskFilter = LandsatMaskFilterType::New();
        landsatMaskFilter->SetInputQualityMask(maskReaderQuality->GetOutput());
        landsatMaskFilter->SetInputCloudsMask(maskReaderCloud->GetOutput());

        // Add the filter to the list
        m_LandsatMaskFilters->PushBack(landsatMaskFilter);

        // Resample if needed
        return getResampledBands(landsatMaskFilter->GetOutput(), td, curRes, true);
    }

    ImageType::Pointer getSentinelBands(const MACCSFileMetadata& meta, const std::string& rootFolder, const TileData& td) {
        // Return the bands associated with a SENTINEL product
        // get the bands
        //Extract the first 3 bands form the first file.
        std::string imageFile1 = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R1");
        ImageReaderType::Pointer reader1 = getReader(imageFile1);
        reader1->UpdateOutputInformation();
        float curRes1 = reader1->GetOutput()->GetSpacing()[0];

        // Get the index of the green band
        int gIndex = getBandIndex(meta.ImageInformation.Resolutions[0].Bands, "B3");

        // Get the index of the red band
        int rIndex = getBandIndex(meta.ImageInformation.Resolutions[0].Bands, "B4");

        // Get the index of the NIR band
        int nirIndex = getBandIndex(meta.ImageInformation.Resolutions[0].Bands, "B8");

        MultiChannelExtractROIType::Pointer chExtractor1 = MultiChannelExtractROIType::New();
        chExtractor1->SetInput(reader1->GetOutput());
        chExtractor1->SetChannel(gIndex);
        chExtractor1->SetChannel(rIndex);
        chExtractor1->SetChannel(nirIndex);
        m_ChannelExtractors->PushBack(chExtractor1);


        //Extract the last band form the second file.
        std::string imageFile2 = getMACCSRasterFileName(rootFolder, meta.ProductOrganization.ImageFiles, "_FRE_R2");
        ImageReaderType::Pointer reader2 = getReader(imageFile2);
        reader2->UpdateOutputInformation();
        float curRes2 = reader2->GetOutput()->GetSpacing()[0];

        // Get the index of the SWIR band
        int swirIndex = getBandIndex(meta.ImageInformation.Resolutions[1].Bands, "B11");
        MultiChannelExtractROIType::Pointer chExtractor2 = MultiChannelExtractROIType::New();
        chExtractor2->SetInput(reader2->GetOutput());
        chExtractor2->SetChannel(swirIndex);
        m_ChannelExtractors->PushBack(chExtractor2);


        // Concatenate the previous results resampling if needed
        ConcatenateVectorImagesFilterType::Pointer concat = ConcatenateVectorImagesFilterType::New();
        concat->PushBackInput(getResampledBands(chExtractor1->GetOutput(), td, curRes1, false));
        concat->PushBackInput(getResampledBands(chExtractor2->GetOutput(), td, curRes2, false));
        m_ImageMergers->PushBack(concat);

        // Return the concatenation result
        return concat->GetOutput();
    }

    ImageType::Pointer getSentinelMask(const MACCSFileMetadata& meta, const std::string& rootFolder, const TileData& td) {
        // Return the mask associated with a SENTINEL product

        // Get the quality mask
        std::string maskFileQuality = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_QLT_R1");
        ImageReaderType::Pointer maskReaderQuality = getReader(maskFileQuality);
        maskReaderQuality->UpdateOutputInformation();
        float curRes = maskReaderQuality->GetOutput()->GetSpacing()[0];

         // Get the cloud mask
        std::string maskFileCloud = getMACCSMaskFileName(rootFolder, meta.ProductOrganization.AnnexFiles, "_CLD_R1");
        ImageReaderType::Pointer maskReaderCloud = getReader(maskFileCloud);

        // Build the SentinelMaskFilter
        SentinelMaskFilterType::Pointer sentinelMaskFilter = SentinelMaskFilterType::New();
        sentinelMaskFilter->SetInputQualityMask(maskReaderQuality->GetOutput());
        sentinelMaskFilter->SetInputCloudsMask(maskReaderCloud->GetOutput());

        // Add the filter to the list
        m_SentinelMaskFilters->PushBack(sentinelMaskFilter);

        // Resample if needed
        return getResampledBands(sentinelMaskFilter->GetOutput(), td, curRes, true);
    }

    ImageType::Pointer getResampledBands(const ImageType::Pointer& image, const TileData& td, const float curRes, const bool isMask) {
         const float invRatio = static_cast<float>(m_pixSize) / curRes;

         // Scale Transform
         OutputVectorType scale;
         scale[0] = invRatio;
         scale[1] = invRatio;

         image->UpdateOutputInformation();
         auto imageSize = image->GetLargestPossibleRegion().GetSize();

         // Evaluate size
         ResampleFilterType::SizeType recomputedSize;
         if (td.m_imageWidth != 0 && td.m_imageHeight != 0) {
             recomputedSize[0] = td.m_imageWidth;
             recomputedSize[1] = td.m_imageHeight;
         } else {
             recomputedSize[0] = imageSize[0] / scale[0];
             recomputedSize[1] = imageSize[1] / scale[1];
         }

         if (imageSize == recomputedSize)
             return image;

         ResampleFilterType::Pointer resampler = ResampleFilterType::New();
         resampler->SetInput(image);

         // Set the interpolator
         if (isMask) {
             NearestNeighborInterpolationType::Pointer interpolator = NearestNeighborInterpolationType::New();
             resampler->SetInterpolator(interpolator);
         } else {
             BicubicInterpolationType::Pointer interpolator = BicubicInterpolationType::New();
             resampler->SetInterpolator(interpolator);
         }

  //       IdentityTransformType::Pointer transform = IdentityTransformType::New();

         resampler->SetOutputParametersFromImage( image );

         // Evaluate spacing
         ImageType::SpacingType spacing = image->GetSpacing();
         ImageType::SpacingType OutputSpacing;
         OutputSpacing[0] = spacing[0] * scale[0];
         OutputSpacing[1] = spacing[1] * scale[1];

         resampler->SetOutputSpacing(OutputSpacing);

  //       FloatVectorImageType::PointType origin = image->GetOrigin();
  //       FloatVectorImageType::PointType outputOrigin;
  //       outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
  //       outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

         resampler->SetOutputOrigin(td.m_imageOrigin);
         resampler->SetCheckOutputBounds(false);

  //       resampler->SetTransform(transform);

         resampler->SetOutputSize(recomputedSize);

         // Padd with nodata
         ImageType::PixelType defaultValue;
         itk::NumericTraits<ImageType::PixelType>::SetLength(defaultValue, image->GetNumberOfComponentsPerPixel());
         if(!isMask) {
             defaultValue = -10000;
         }
         resampler->SetEdgePaddingValue(defaultValue);


         m_ResamplersList->PushBack(resampler);
         return resampler->GetOutput();
    }


    // build the date for spot products as YYYYMMDD
    inline std::string formatSPOT4Date(const std::string& date) {
        return date.substr(0,4) + date.substr(5,2) + date.substr(8,2);
    }
};

template<typename TTimeSeriesReaderList>
std::map<std::string, std::vector<int>> getOutputDays(TTimeSeriesReaderList preprocessors, bool resample, const std::map<std::string, int> &sp)
{
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
            auto it = sp.find(sensor.first);
            if (it == sp.end()) {
                itkGenericExceptionMacro("Sampling rate required for sensor " << sensor.first);
            }
            auto rate = it->second;

            auto last = *sensor.second.rbegin();
            for (int date = *sensor.second.begin(); date <= last; date += rate) {
                outDates.emplace_back(date);
            }
        } else {
            outDates.insert(outDates.end(), sensor.second.begin(), sensor.second.end());
        }
        sensorOutDays[sensor.first] = outDates;
    }

    return sensorOutDays;
}

void writeOutputDays(const std::map<std::string, std::vector<int>> &days, const std::string &file)
{
    std::ofstream f(file);
    for (const auto &p : days) {
        for (auto d : p.second) {
            f << p.first << ' ' << d << '\n';
        }
    }
}

const std::map<std::string, std::vector<int>> readOutputDays(const std::string &file)
{
    std::map<std::string, std::vector<int>> days;
    std::ifstream f(file);
    std::string mission;
    int day;
    while (f >> mission >> day) {
        days[mission].emplace_back(day);
    }
    return days;
}
