#ifndef TOTALWEIGHTCOMPUTATION_H
#define TOTALWEIGHTCOMPUTATION_H

#include "otbWrapperTypes.h"
#include "itkBinaryFunctorImageFilter.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"

namespace Functor
{
template< class TPixel>
class TotalWeightCalculationFunctor
{
public:
  TotalWeightCalculationFunctor() {}
  ~TotalWeightCalculationFunctor() {}
  bool operator!=(const TotalWeightCalculationFunctor &) const
  {
    return false;
  }

  bool operator==(const TotalWeightCalculationFunctor & other) const
  {
    return !( *this != other );
  }

  void SetFixedWeight(float weightSensor, float weightDate)
  {
    m_fixedWeight = weightSensor * weightDate;
  }

  inline TPixel operator()(const TPixel & A,
                            const TPixel & B) const
  {
    const float dA = static_cast< float >( A );
    const float dB = static_cast< float >( B );
    const float totalWeight = fabs(dA * dB * m_fixedWeight);

    return static_cast< TPixel >( totalWeight );
  }
private:
  float m_fixedWeight;
};
}

class TotalWeightComputation
{
public:
    typedef otb::Wrapper::FloatImageType ImageType;
    typedef enum {S2A, S2B, L8, UNKNOWN} SensorType;
    typedef itk::BinaryFunctorImageFilter< ImageType, ImageType, ImageType,
                              Functor::TotalWeightCalculationFunctor<ImageType::PixelType> > FilterType;
    typedef otb::ImageFileReader<ImageType> ReaderType;
    typedef otb::ImageFileWriter<ImageType> WriterType;

    typedef itk::ImageSource<ImageType> ImageSource;
    typedef FilterType::Superclass::Superclass OutImageSource;

public:
    TotalWeightComputation();

    void SetMissionName(std::string &missionName);
    void SetWeightOnSensor(float fWeight);
    void SetDates(std::string &L2ADate, std::string &L3ADate);
    void SetHalfSynthesisPeriodAsDays(int deltaMax);
    void SetWeightOnDateMin(float fMinWeight);
    void SetAotWeightFile(std::string &aotWeightFileName);
    void SetCloudsWeightFile(std::string &cloudsWeightFileName);
    void SetTotalWeightOutputFileName(std::string &outFileName);

    const char *GetNameOfClass() { return "TotalWeightComputation";}
    OutImageSource::Pointer GetOutputImageSource();

    void WriteToOutputFile();

protected:
    void ComputeTotalWeight();
    void ComputeWeightOnSensor();
    void ComputeWeightOnDate();

protected:
    float m_fWeightOnSensor;
    float m_fWeightOnDate;

private:
    void BuildOutputImageSource();

    SensorType m_sensorType;
    int m_nDaysTimeInterval;
    int m_nDeltaMax;
    float m_fWeightOnDateMin;
    std::string m_strOutFileName;

    ImageSource::Pointer m_inputReader1;
    ImageSource::Pointer m_inputReader2;

    FilterType::Pointer m_filter;
};

#endif // TOTALWEIGHTCOMPUTATION_H
