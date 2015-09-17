#ifndef CLOUDMASKBINARIZATION_H
#define CLOUDMASKBINARIZATION_H

#include "otbWrapperTypes.h"
#include "itkUnaryFunctorImageFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkImageSource.h"

namespace Functor
{
template< class TInput, class TOutput>
class BinarizeCloudMask
{
public:
    BinarizeCloudMask() {}
    ~BinarizeCloudMask() {}
  bool operator!=( const BinarizeCloudMask & ) const
    {
    return false;
    }
  bool operator==( const BinarizeCloudMask & other ) const
    {
    return !(*this != other);
    }
  inline TOutput operator()( const TInput & A ) const
    {
      int val = static_cast< int >( A );

      return ((val == -10000) ? -10000 : ((val == 0) ? 0.0 : 1.0));
    }
};
}

class CloudMaskBinarization
{
public:
    typedef otb::Wrapper::UInt8ImageType ImageType;
    typedef otb::Wrapper::FloatImageType OutImageType;

    typedef itk::UnaryFunctorImageFilter<ImageType,OutImageType,
                    Functor::BinarizeCloudMask<
                        ImageType::PixelType,
                        OutImageType::PixelType> > FilterType;


    typedef itk::ImageSource<ImageType> ImageSource;
    typedef FilterType::Superclass::Superclass::Superclass OutImageSource;

    typedef otb::ImageFileReader<ImageType> ReaderType;
    typedef otb::ImageFileWriter<OutImageType> WriterType;

public:
    CloudMaskBinarization();

    void SetInputFileName(std::string &inputImageStr);
    void SetInputImageReader(ImageSource::Pointer inputReader);
    void SetOutputFileName(std::string &outFile);

    const char *GetNameOfClass() { return "CloudMaskBinarization";}
    OutImageSource::Pointer GetOutputImageSource();
    int GetInputImageResolution();
    void WriteToOutputFile();

private:
    void BuildOutputImageSource();
    ImageSource::Pointer m_inputReader;
    std::string m_outputFileName;
    FilterType::Pointer m_filter;
};

#endif // CLOUDMASKBINARIZATION_H
