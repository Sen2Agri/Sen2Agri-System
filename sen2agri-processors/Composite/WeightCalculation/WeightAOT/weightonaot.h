#ifndef WEIGHTONAOT_H
#define WEIGHTONAOT_H

#include "otbWrapperTypes.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkImageSource.h"
#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbObjectList.h"


class WeightOnAOT
{
public:
    typedef otb::Wrapper::FloatVectorImageType ImageType;
    typedef otb::Wrapper::FloatImageType OutImageType;

    typedef itk::ImageSource<ImageType> ImageSource;

    typedef otb::ImageFileReader<ImageType> ReaderType;
    typedef otb::ImageFileWriter<OutImageType> WriterType;

    typedef otb::MultiToMonoChannelExtractROI<ImageType::InternalPixelType,
                       otb::Wrapper::FloatImageType::PixelType>    ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                  ExtractROIFilterListType;
    typedef otb::BandMathImageFilter<otb::Wrapper::FloatImageType>  BandMathImageFilterType;

    typedef BandMathImageFilterType::Superclass::Superclass::Superclass OutImageSource;

public:
    WeightOnAOT();

    void SetInputFileName(std::string &inputImageStr);
    void SetInputImage(ImageType::Pointer image);
    void SetInputImageReader(ImageSource::Pointer inputReader);
    void SetOutputFileName(std::string &outFile);

    void SetBand(int band);
    void SetAotQuantificationValue(float fQuantifVal);
    void SetAotMaxValue(int nMaxAot);
    void SetMinAotWeight(float fMinWeightAot);
    void SetMaxAotWeight(float fMaxWeightAot);

    const char *GetNameOfClass() { return "CloudMaskBinarization";}
    OutImageType::Pointer GetProducedImage();
    OutImageSource::Pointer GetOutputImageSource();

    void Update();
    void WriteToOutputFile();

private:
    ImageSource::Pointer m_inputReader;
    ImageType::Pointer m_image;
    std::string m_outputFileName;
    ExtractROIFilterType::Pointer     m_ExtractROIFilter;
    ExtractROIFilterListType::Pointer m_ChannelExtractorList;
    BandMathImageFilterType::Pointer  m_Filter;

    int m_nBand;
    float m_fAotQuantificationVal;
    int m_nAotMax;
    float m_fMinWeightAot;
    float m_fMaxWeightAot;

};

#endif // WEIGHTONAOT_H
