#ifndef RESAMPLED_BAND_EXTRACTOR_H
#define RESAMPLED_BAND_EXTRACTOR_H

#include "otbWrapperTypes.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbImageListToVectorImageFilter.h"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"

#include "libgen.h"

//Transform
#include "itkScalableAffineTransform.h"

class ResampledBandExtractor
{
public:
    typedef float                                      PixelType;
    typedef otb::VectorImage<PixelType, 2>             ImageType;
    typedef otb::Image<PixelType, 2>                   InternalImageType;
    typedef otb::ImageList<ImageType>                  ImageListType;
    typedef otb::ImageList<InternalImageType>          InternalImageListType;

    typedef otb::MultiToMonoChannelExtractROI<ImageType::InternalPixelType,
                                              InternalImageType::PixelType>     ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                               ExtractROIFilterListType;

    typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;

    typedef otb::StreamingResampleImageFilter<InternalImageType, InternalImageType, double>     ResampleFilterType;
    typedef otb::ObjectList<ResampleFilterType>                                                 ResampleFilterListType;

    typedef itk::NearestNeighborInterpolateImageFunction<InternalImageType, double>             NearestNeighborInterpolationType;
    typedef itk::LinearInterpolateImageFunction<InternalImageType, double>                      LinearInterpolationType;
    typedef otb::BCOInterpolateImageFunction<InternalImageType>                                 BCOInterpolationType;
    typedef itk::IdentityTransform<double, InternalImageType::ImageDimension>                   IdentityTransformType;

    typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension>             ScalableTransformType;

    typedef ScalableTransformType::OutputVectorType     OutputVectorType;

public:
    ResampledBandExtractor();
    InternalImageType::Pointer ExtractResampledBand(ImageType::Pointer img, int nChannel, int curRes=-1,
                                                  int nDesiredRes=-1, bool bNearestNeighbourInterpolation=false);

    const char * GetNameOfClass() { return "MasksBandExtractor"; }

    ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const float& ratio, bool isMask=false);
    ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const int wantedWidth, const int wantedHeight, bool isMask=false);
    ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const OutputVectorType& scale, bool isMask=false);

private:
    InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes,
                                                 ExtractROIFilterType::Pointer extractor,
                                                 bool bIsMask);
private:
    bool m_bAllInOne;
    std::string m_strXml;
    std::string m_strMaskFileName;

    ExtractROIFilterListType::Pointer     m_ExtractorList;
    ImageReaderListType::Pointer          m_ImageReaderList;
    ResampleFilterListType::Pointer       m_ResamplersList;
};

#endif // RESAMPLED_BAND_EXTRACTOR_H
