#ifndef CLOUDSINTERPOLATION_H
#define CLOUDSINTERPOLATION_H

#include "otbVectorImage.h"
#include "otbWrapperTypes.h"
#include "otbBCOInterpolateImageFunction.h"
#include "itkNearestNeighborInterpolateImageFunction.h"

//Transform
#include "otbCompositeTransform.h"
#include "itkScalableAffineTransform.h"
#include "itkTranslationTransform.h"
#include "itkIdentityTransform.h"
#include "itkScaleTransform.h"
#include "itkCenteredRigid2DTransform.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"

#include "otbStreamingResampleImageFilter.h"

typedef enum
{
  Interpolator_NNeighbor,
  Interpolator_Linear,
  Interpolator_BCO
} Interpolator_Type;

class CloudsInterpolation
{
public:
    typedef otb::Wrapper::FloatImageType ImageType;
    typedef otb::ImageFileReader<ImageType> ReaderType;
    typedef otb::ImageFileWriter<ImageType> WriterType;
    typedef otb::StreamingResampleImageFilter<ImageType, ImageType, double>    ResampleFilterType;
    typedef itk::IdentityTransform<double, ImageType::ImageDimension>      IdentityTransformType;
    typedef itk::ScalableAffineTransform<double, ImageType::ImageDimension> ScalableTransformType;
    typedef ScalableTransformType::OutputVectorType                         OutputVectorType;

    typedef itk::ImageSource<ImageType> ImageSource;
    typedef ResampleFilterType::Superclass::Superclass OutImageSource;

public:
    CloudsInterpolation();

    void SetInputFileName(std::string &inputImageStr);
    void SetOutputFileName(std::string &outFile);
    void SetInputImageReader(ImageSource::Pointer inputReader);
    void SetInputImage(const ImageType::Pointer inputImage);
    void SetInputResolution(int inputRes);
    void SetOutputResolution(int outputRes);
    void SetInterpolator(Interpolator_Type interpolator);
    void SetBicubicInterpolatorRadius(int bcoRadius);

    const char *GetNameOfClass() { return "CloudsInterpolation";}
    const ImageType::Pointer GetProducedImage();
    OutImageSource::Pointer GetOutputImageSource();

    void Update();
    void WriteToOutputFile();

private:

    ImageType::Pointer m_inputImage;
    ImageSource::Pointer m_inputReader;
    int m_inputRes;
    int m_outputRes;
    unsigned int m_BCORadius;
    int m_interpolator;
    ResampleFilterType::Pointer m_Resampler;

    std::string m_outputFileName;
};
#endif // CLOUDSINTERPOLATION_H
