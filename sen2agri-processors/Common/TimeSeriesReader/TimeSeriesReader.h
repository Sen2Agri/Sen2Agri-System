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

// Filters
#include "itkVectorIndexSelectionCastImageFilter.h"

#include "otbSpotMaskFilter.h"
#include "otbSentinelMaskFilter.h"

#include <string>
#include <map>

#include <boost/filesystem.hpp>
#include "MetadataHelper.h"

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

#define LANDSAT    "LANDSAT"
#define SENTINEL   "SENTINEL"
#define SPOT       "SPOT"

#define MASK_TYPE_NUA   0
#define MASK_TYPE_SAT   1
#define MASK_TYPE_DIV   2

int getDaysFromEpoch(const std::string &date);

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
    // And possibly the red edge ones
    // S2: B3, B4, B8, B11, possibly B5, B6, B7, B8a
    std::vector<otb::Wrapper::FloatImageType::Pointer>                bands;
    // S2: B2, B4, B5, B6, B7, B8
    std::vector<otb::Wrapper::FloatImageType::Pointer>                redEdgeBands;
    // The combined mask
    otb::Wrapper::UInt8ImageType::Pointer          mask;

    // the format of the metadata
    unsigned char type;
    // the metadata
    //SourceImageMetadata metadata;
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

    void Build(std::vector<std::string>::const_iterator begin, std::vector<std::string>::const_iterator end, const TileData &td);

    void updateRequiredImageSize(const std::vector<std::string>& descriptors, int startIndex, int endIndex, TileData& td);
    void SetMission(std::string mission);
    const std::string & GetMission() const;
    void SetPixelSize(float pixelSize);
    float GetPixelSize() const;
    void SetDescriptorList(std::vector<ImageDescriptor> descriptors);
    const std::vector<ImageDescriptor> & GetDescriptorList() const;
    void SetSamplingRates(std::map<std::string, int> samplingRates);
    const std::map<std::string, int> & GetSamplingRates() const;

protected:
    std::string m_mission;
    float m_pixSize;
    std::vector<ImageDescriptor> m_Descriptors;
    std::vector<std::unique_ptr<MetadataHelper<float, uint8_t>>> m_ProductReaders;
    std::map<std::string, int> m_SamplingRates;
    std::map<std::string, std::vector<int> > m_SensorOutDays;

    ImageReaderListType::Pointer             m_ImageReaderList;
    UInt8ImageReaderListType::Pointer        m_UInt8ImageReaderList;
    Int16ImageReaderListType::Pointer        m_Int16ImageReaderList;
    UInt16ImageReaderListType::Pointer       m_UInt16ImageReaderList;
    UInt8VectorImageReaderListType::Pointer  m_UInt8VectorImageReaderList;

    CastInt16FloatFilterListType::Pointer        m_CastInt16FloatFilterFilst;

//    SpotMaskFilterListType::Pointer                   m_SpotMaskFilters;
//    SentinelMaskFilterListType::Pointer               m_SentinelMaskFilters;
    ExtractChannelListType::Pointer                   m_ChannelExtractors;

    FloatImageListType::Pointer                       m_FloatImageList;
    UInt8ImageListType::Pointer                       m_UInt8ImageList;
    FloatImageListType::Pointer                       m_RedEdgeBandImageList;
    UInt8ImageListType::Pointer                       m_RedEdgeMaskImageList;
    ConcatenateFloatImagesFilterType::Pointer         m_BandsConcat;
    ConcatenateUInt8ImagesFilterType::Pointer         m_MaskConcat;
    ConcatenateFloatImagesFilterType::Pointer         m_RedEdgeBandConcat;
    ConcatenateUInt8ImagesFilterType::Pointer         m_RedEdgeMaskConcat;

    TimeSeriesReader();
    virtual ~TimeSeriesReader();

    // Sort the descriptors based on the aquisition date
    static bool SortUnmergedMetadata(const ImageDescriptor& o1, const ImageDescriptor& o2);

    void ProcessMetadata(const std::unique_ptr<MetadataHelper<float, uint8_t>>& pHelper, const std::string& filename,
                         ImageDescriptor& descriptor, const TileData& td);
    virtual void GetProductBands(const std::unique_ptr<MetadataHelper<float, uint8_t>>& pHelper, const TileData& td,
                         ImageDescriptor &descriptor);
    otb::Wrapper::UInt8ImageType::Pointer GetProductMask(const std::unique_ptr<MetadataHelper<float, uint8_t>>& pHelper,
                                                         const TileData& td);
    virtual void getRedEdgeBands(const std::unique_ptr<MetadataHelper<float, uint8_t>>&,
                                const TileData &td,
                                ImageDescriptor &descriptor);

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
//        std::cerr << image->GetOrigin() << '\n';
//        std::cerr << td.m_imageOrigin << '\n';
        bool sameOrigin = image->GetOrigin() == td.m_imageOrigin;
//        std::cerr << sameOrigin << '\n';
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

    template<typename ImageType>
    typename ImageType::Pointer getResampledBand2(const typename ImageType::Pointer& image, const TileData& td, bool isMask)
    {
        image->UpdateOutputInformation();
        auto imageSize = image->GetLargestPossibleRegion().GetSize();


        // Evaluate size
        typename ImageType::SizeType recomputedSize;
        recomputedSize[0] = td.m_imageWidth;
        recomputedSize[1] = td.m_imageHeight;

        std::string inputProjection = image->GetProjectionRef();
        bool sameProjection = inputProjection == td.m_projection;
        std::cerr << image->GetOrigin() << '\n';
        std::cerr << td.m_imageOrigin << '\n';
        bool sameOrigin = image->GetOrigin() == td.m_imageOrigin;
        std::cerr << sameOrigin << '\n';
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
//    inline std::string formatSPOT4Date(const std::string& date) {
//        return date.substr(0,4) + date.substr(5,2) + date.substr(8,2);
//    }

    virtual int getBandCount(const std::string &sensor) {
        if (sensor == "SENTINEL" || sensor == "SPOT" || sensor == "LANDSAT") {
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

enum class TemporalResamplingMode {
    GapFill,
    Resample,
    GapFillMainMission,
};

template<typename TTimeSeriesReaderList>
std::vector<MissionDays> getOutputDays(TTimeSeriesReaderList preprocessors, TemporalResamplingMode resamplingMode, const std::string &mainMission, const std::vector<SensorPreferences> &sp)
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

    std::vector<int> mainMissionDays;
    if (resamplingMode == TemporalResamplingMode::GapFillMainMission) {
        const auto &days = sensorInDays[mainMission];
        mainMissionDays.assign(days.begin(), days.end());
    }

    // loop through the sensors to determinte the output dates
    for (const auto& sensor : sensorInDays) {
        std::vector<int> outDates;
        if (resamplingMode == TemporalResamplingMode::Resample) {
            auto it = spMap.find(sensor.first);
            if (it == spMap.end()) {
                itkGenericExceptionMacro("Sampling rate required for sensor " << sensor.first);
            }
            auto rate = it->second.samplingRate;

            auto last = *sensor.second.rbegin();
            for (int date = *sensor.second.begin(); date <= last; date += rate) {
                outDates.emplace_back(date);
            }
        } else if (resamplingMode == TemporalResamplingMode::GapFill) {
            outDates.assign(sensor.second.begin(), sensor.second.end());
        } else if (resamplingMode == TemporalResamplingMode::GapFillMainMission) {
            outDates = mainMissionDays;
        }
        sensorOutDays[sensor.first] = std::move(outDates);
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

void writeOutputDays(const std::vector<MissionDays> &days, const std::string &file);

std::vector<SensorPreferences> parseSensorPreferences(const std::vector<std::string> &sp);

const std::vector<MissionDays> readOutputDays(const std::string &file);
