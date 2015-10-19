#ifndef RESAMPLING_BAND_EXTRACTOR_H
#define RESAMPLING_BAND_EXTRACTOR_H

#include "otbWrapperTypes.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbImageListToVectorImageFilter.h"

#include "libgen.h"

//Transform
#include "itkScalableAffineTransform.h"

class ResamplingBandExtractor
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
    ResamplingBandExtractor();
    InternalImageType::Pointer ExtractResampledBand(const ImageType::Pointer img, int nChannel, int curRes=-1,
                                                  int nDesiredRes=-1, int nForcedOutWidth=-1, int nForcedOutHeight=-1,
                                                  bool bNearestNeighbourInterpolation=false);

    int ExtractAllResampledBands(const ImageType::Pointer img, otb::ImageList<otb::Wrapper::FloatImageType>::Pointer &outList,
                                                  int curRes=-1, int nDesiredRes=-1, bool bNearestNeighbourInterpolation=false);

    const char * GetNameOfClass() { return "ResamplingBandExtractor"; }

    ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const float& ratio, bool isMask=false);
    ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const int wantedWidth, const int wantedHeight, bool isMask=false);
    ResampleFilterType::Pointer getResampler(const InternalImageType::Pointer& image, const OutputVectorType& scale, int forcedWidth, int forcedHeight, bool isMask=false);

private:
    InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes, int forcedWidth, int forcedHeight,
                                                 ExtractROIFilterType::Pointer extractor,
                                                 bool bIsMask);
private:
    ExtractROIFilterListType::Pointer     m_ExtractorList;
    ImageReaderListType::Pointer          m_ImageReaderList;
    ResampleFilterListType::Pointer       m_ResamplersList;
};

#endif // RESAMPLING_BAND_EXTRACTOR_H
