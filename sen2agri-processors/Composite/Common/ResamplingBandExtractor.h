#ifndef RESAMPLING_BAND_EXTRACTOR_H
#define RESAMPLING_BAND_EXTRACTOR_H

#include "otbWrapperTypes.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageListToVectorImageFilter.h"

#include "libgen.h"

//Transform
#include "itkScalableAffineTransform.h"

#include "ImageResampler.h"

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

    typedef itk::ScalableAffineTransform<double, InternalImageType::ImageDimension>             ScalableTransformType;
    typedef typename ScalableTransformType::OutputVectorType     OutputVectorType;

public:
    ResamplingBandExtractor();
    InternalImageType::Pointer ExtractResampledBand(const std::string &fileName, int nChannel,
                                                  int nCurRes = -1, int nDesiredRes=-1, int nForcedOutWidth=-1, int nForcedOutHeight=-1,
                                                  Interpolator_Type interpolator=Interpolator_Linear);

    int ExtractAllResampledBands(const std::string &fileName, otb::ImageList<otb::Wrapper::FloatImageType>::Pointer &outList,
                                int nCurRes = -1, int nDesiredRes=-1, int nForcedOutWidth=-1, int nForcedOutHeight=-1,
                                Interpolator_Type interpolator=Interpolator_Linear);

    InternalImageType::Pointer ExtractResampledBand(const ImageType::Pointer img, int nChannel, int curRes=-1,
                                                  int nDesiredRes=-1, int nForcedOutWidth=-1, int nForcedOutHeight=-1,
                                                  Interpolator_Type interpolator=Interpolator_Linear);

    int ExtractAllResampledBands(const ImageType::Pointer img, otb::ImageList<otb::Wrapper::FloatImageType>::Pointer &outList,
                                int curRes=-1, int nDesiredRes=-1, int nForcedOutWidth=-1, int nForcedOutHeight=-1,
                                Interpolator_Type interpolator=Interpolator_Linear);

    const char * GetNameOfClass() { return "ResamplingBandExtractor"; }

private:
    InternalImageType::Pointer getResampledImage(int nCurRes, int nDesiredRes, int forcedWidth, int forcedHeight,
                                                 ExtractROIFilterType::Pointer extractor,
                                                 Interpolator_Type interpolator);
private:
    ExtractROIFilterListType::Pointer     m_ExtractorList;
    ImageReaderListType::Pointer          m_ImageReaderList;
    ImageResampler<InternalImageType, InternalImageType>     m_ImageResampler;
};

#endif // RESAMPLING_BAND_EXTRACTOR_H



