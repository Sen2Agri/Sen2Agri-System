#ifndef CREATE_S2_ANGLES_RASTER_H
#define CREATE_S2_ANGLES_RASTER_H

#include "otbWrapperTypes.h"
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"

#include "MACCSMetadataReader.hpp"
#include "ViewingAngles.hpp"

#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkCastImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include "otbStreamingResampleImageFilter.h"
//Transform
#include "itkScalableAffineTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"

#include "itkResampleImageFilter.h"
#include "itkIndent.h"
#include <vector>
#include "libgen.h"

class CreateS2AnglesRaster
{
public:
    typedef float                                     PixelType;
    typedef otb::Wrapper::FloatVectorImageType                  OutputImageType;
    typedef otb::ImageList<OutputImageType>            ImageListType;
    typedef otb::ImageFileWriter<OutputImageType>      WriterType;
    typedef otb::MultiToMonoChannelExtractROI<otb::Wrapper::FloatVectorImageType::InternalPixelType,
                                              otb::Wrapper::FloatImageType::PixelType>    ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                           ExtractROIFilterListType;
    typedef otb::BandMathImageFilter<otb::Wrapper::Int16ImageType>   BMFilterType;

    typedef itk::MACCSMetadataReader                                   MACCSMetadataReaderType;
    typedef otb::StreamingResampleImageFilter<OutputImageType, OutputImageType, double>    ResampleFilterType;
    typedef itk::LinearInterpolateImageFunction<OutputImageType,  double>          LinearInterpolationType;
    typedef itk::IdentityTransform<double, OutputImageType::ImageDimension>      IdentityTransformType;
    typedef itk::ScalableAffineTransform<double, OutputImageType::ImageDimension> ScalableTransformType;
    typedef ScalableTransformType::OutputVectorType                         OutputVectorType;

public:
    CreateS2AnglesRaster();
    void DoInit(int res, std::string &xml);
    OutputImageType::Pointer DoExecute();
    const char * GetNameOfClass() { return "CreateS2AnglesRaster"; }

private:
    void createResampler(const OutputImageType::Pointer& image, const int wantedWidth, const int wantedHeight);
    std::string getMACCSRasterFileName(const std::string& rootFolder,
                                       const std::vector<MACCSFileInformation>& imageFiles,
                                       const std::string& ending,
                                       const bool fileTypeMeta);

private:
    OutputImageType::Pointer            m_AnglesRaster;
    ResampleFilterType::Pointer         m_Resampler;
    std::string                         m_DirName;
    std::string                         m_inXml;
    int                                 m_nOutRes;

};

#endif // CREATE_S2_ANGLES_RASTER_H
