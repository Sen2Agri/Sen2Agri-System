#ifndef CLOUDWEIGHTCOMPUTATION_H
#define CLOUDWEIGHTCOMPUTATION_H

#include "otbWrapperTypes.h"
#include "itkBinaryFunctorImageFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"

namespace Functor
{
template< class TPixel>
class WeightOnCloudsCalculation
{
public:
  WeightOnCloudsCalculation() {}
  ~WeightOnCloudsCalculation() {}
  bool operator!=(const WeightOnCloudsCalculation &) const
  {
    return false;
  }

  bool operator==(const WeightOnCloudsCalculation & other) const
  {
    return !( *this != other );
  }

  inline TPixel operator()(const TPixel & A,
                            const TPixel & B) const
  {
    const float dA = static_cast< float >( A );
    const float dB = static_cast< float >( B );
    const float weight = (1-dA) * (1 - dB);

    return static_cast< TPixel >( weight );
  }
};
}

class CloudWeightComputation
{
public:
    typedef otb::Wrapper::FloatImageType ImageType;
    typedef itk::BinaryFunctorImageFilter< ImageType, ImageType, ImageType,
                              Functor::WeightOnCloudsCalculation<ImageType::PixelType> > FilterType;
    typedef otb::ImageFileReader<ImageType> ReaderType;
    typedef otb::ImageFileWriter<ImageType> WriterType;

    typedef itk::ImageSource<ImageType> ImageSource;
    typedef FilterType::Superclass::Superclass OutImageSource;

public:
    CloudWeightComputation();

    void SetInputFileName1(std::string &inputImageStr);
    void SetInputFileName2(std::string &inputImageStr);
    void SetInputImageReader1(ImageSource::Pointer inputReader);
    void SetInputImageReader2(ImageSource::Pointer inputReader);
    void SetOutputFileName(std::string &outFile);

    const char *GetNameOfClass() { return "WeightOnClouds";}
    OutImageSource::Pointer GetOutputImageSource();
    void WriteToOutputFile();

private:
    void BuildOutputImageSource();
    ImageSource::Pointer m_inputReader1;
    ImageSource::Pointer m_inputReader2;
    std::string m_outputFileName;
    FilterType::Pointer m_filter;
};

#endif // WEIGHTONCLOUDS_H
