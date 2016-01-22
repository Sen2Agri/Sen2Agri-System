#ifndef CLOUDSINTERPOLATION_H
#define CLOUDSINTERPOLATION_H

#include "otbVectorImage.h"
#include "otbWrapperTypes.h"

//Transform
#include "otbImageFileWriter.h"
#include "itkZeroFluxNeumannPadImageFilter.h"

template <typename TInput1, typename TInput2, typename TOutput>
class PaddingImageHandler
{
public:

    typedef itk::ScalableAffineTransform<double, TInput2::ImageDimension>             ScalableTransformType;
    typedef typename ScalableTransformType::OutputVectorType     OutputVectorType;

    typedef otb::ImageFileWriter<TOutput> WriterType;
    typedef itk::ImageSource<TInput1> ImageSource;
    typedef itk::ImageSource<TInput2> ResampledImageSource;
    typedef itk::ZeroFluxNeumannPadImageFilter<TInput2, TOutput> EdgePaddingResizer;

public:
    PaddingImageHandler() {
        m_outForcedWidth = -1;
        m_outForcedHeight = -1;
    }

    void SetOutputFileName(std::string &outFile) {
        m_outputFileName = outFile;
    }

    void SetInputImageReader(TInput1::Pointer inputImg, TInput2::Pointer resampledImg) {
        if (inputImg>IsNull() || resampledImg->IsNull())
        {
            std::cout << "No input Image set...; please set the input image!" << std::endl;
            itkExceptionMacro("No input Image set...; please set the input image");
        }
        m_inputImg = inputReader;
        m_resampledImg = resampledImg;
    }

    const char *GetNameOfClass() { return "CloudsInterpolation";}

    typename TOutput::Pointer GetOutputImage() {
        BuildOutputImage();
        return (typename TOutput::Pointer)m_resizer->GetOutput();
    }

    void WriteToOutputFile() {
        if(!m_outputFileName.empty())
        {
            typename WriterType::Pointer writer;
            writer = WriterType::New();
            writer->SetFileName(m_outputFileName);
            writer->SetInput(GetOutputImageSource()->GetOutput());
            try
            {
                writer->Update();
                typename TOutput::SpacingType spacing = m_resampledImg->GetSpacing();
                typename TOutput::PointType origin = m_resampledImg->GetOrigin();
                std::cout << "=================================" << std::endl;
                std::cout << "Origin : " << origin[0] << " " << origin[1] << std::endl;
                std::cout << "Spacing : " << spacing[0] << " " << spacing[1] << std::endl;
                std::cout << "Size : " << m_resampledImg->GetLargestPossibleRegion().GetSize()[0] << " " <<
                             m_resampledImg->GetLargestPossibleRegion().GetSize()[1] << std::endl;

                typename TOutput::SpacingType outspacing = m_resizer->GetOutput()->GetSpacing();
                typename TOutput::PointType outorigin = m_resizer->GetOutput()->GetOrigin();
                std::cout << "Output Origin : " << outorigin[0] << " " << outorigin[1] << std::endl;
                std::cout << "Output Spacing : " << outspacing[0] << " " << outspacing[1] << std::endl;
                std::cout << "Size : " << m_Resampler->GetOutput()->GetLargestPossibleRegion().GetSize()[0] << " " <<
                             m_resizer->GetOutput()->GetLargestPossibleRegion().GetSize()[1] << std::endl;

                std::cout  << "=================================" << std::endl;
                std::cout << std::endl;
            }
            catch (itk::ExceptionObject& err)
            {
                std::cout << "ExceptionObject caught !" << std::endl;
                std::cout << err << std::endl;
                itkExceptionMacro("Error writing output");
            }
        }
    }

private:
    void BuildOutputImage() {
        int inputRes = m_inputImg->GetSpacing()[0];
        int resampledRes = m_resampledImg->GetSpacing()[0];
        float scaleXY = ((float)resampledRes)/((float)inputRes);
        OutputVectorType scale;
        scale[0] = scaleXY;
        scale[1] = scaleXY;

        TInput1::SizeType size;
        size[0] = m_inputImg->GetLargestPossibleRegion().GetSize()[0] / scale[0];
        size[1] = m_inputImg->GetLargestPossibleRegion().GetSize()[1] / scale[1];

        TInput1::SizeType size2;
        size[0] = vcl_ceil(m_inputImg->GetLargestPossibleRegion().GetSize()[0] / scale[0]);
        size[1] = vcl_ceil(m_inputImg->GetLargestPossibleRegion().GetSize()[1] / scale[1]);


        // This will remain to 0 as we consider that the padding will be done only in the uper extend region
        TInput2::SizeType lowerExtendRegion;
        upperExtendRegion[0] = 0;
        upperExtendRegion[1] = 0;

        // This field will be updated for padding
        TInput2::SizeType upperExtendRegion;
        upperExtendRegion[0] = 0;
        upperExtendRegion[1] = 0;

        if(size2[0] > size[0]) {
            upperExtendRegion[0] = (size2[0] - size[0]);
        }
        if(size2[1] > size[1]) {
            // we update only the index 1
            upperExtendRegion[1] = (size2[1] - size[1]);
        }

        if((upperExtendRegion[0] != 0) || (upperExtendRegion[1] != 0)) {
            m_resizer = EdgePaddingResizer::New();
            m_resizer->SetInput(m_resampledImg);
            m_resizer->SetPadLowerBound(lowerExtendRegion);
            m_resizer->SetPadUpperBound(upperExtendRegion);
        }
    }

    typename ImageSource::Pointer m_inputReader;
    typename EdgePaddingResizer::Pointer m_resizer;
    std::string m_outputFileName;
    // during resampling at higher resolutions it might be needed to return with a specified dimension
    long m_outForcedWidth;
    long m_outForcedHeight;

    TInput1::Pointer m_inputImg;
    TInput2::Pointer m_resampledImg;
};
#endif // PaddingImageHandler.h
