#pragma once

#include <itkImageRegionIterator.h>
#include <itkImageToImageFilter.h>
#include <itkProgressReporter.h>

template<class TInputImage, class TOutputImage = TInputImage>
class ITK_EXPORT MaskedScaleImageFilter
  : public itk::ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  typedef MaskedScaleImageFilter                              Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage>  Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  itkNewMacro(Self)

  itkTypeMacro(MaskedScaleImageFilter, ImageToImageFilter)

  /** Template related typedefs */
  typedef TInputImage InputImageType;
  typedef TOutputImage OutputImageType;

  typedef typename InputImageType::Pointer  InputImagePointerType;
  typedef typename OutputImageType::Pointer OutputImagePointerType;

  typedef typename InputImageType::PixelType          InputPixelType;
  typedef typename OutputImageType::InternalPixelType InputInternalPixelType;

  typedef typename OutputImageType::PixelType         OutputPixelType;
  typedef typename OutputImageType::InternalPixelType OutputInternalPixelType;
  typedef typename OutputImageType::RegionType        OutputImageRegionType;

  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TOutputImage::ImageDimension);

  itkSetMacro(NoDataValue, InputInternalPixelType)
  itkGetMacro(NoDataValue, InputInternalPixelType)

  itkSetMacro(Scale, InputPixelType)
  itkGetMacro(Scale, InputPixelType)

protected:
  MaskedScaleImageFilter()
  {
    this->SetNumberOfRequiredInputs(1);
  }

  virtual ~MaskedScaleImageFilter()
  {
  }

  virtual void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId)
  {
    OutputImagePointerType output = this->GetOutput();

    itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

    typename InputImageType::RegionType inputRegionForThread;
    this->CallCopyOutputRegionToInputRegion(inputRegionForThread, outputRegionForThread);

    typedef itk::ImageRegionIterator<InputImageType>  InputIteratorType;
    typedef itk::ImageRegionIterator<OutputImageType> OutputIteratorType;

    InputIteratorType inputIt(const_cast<InputImageType *>(this->GetInput()), inputRegionForThread);
    OutputIteratorType outputIt(output, outputRegionForThread);

    OutputPixelType outputPixel(output->GetVectorLength());

    outputIt.GoToBegin();
    while (!outputIt.IsAtEnd())
      {
      const auto &inputPixel = inputIt.Get();
      for (int i = 0; i < outputPixel.Size(); i++)
        {
        if (inputPixel[i] != m_NoDataValue)
          {
          outputPixel[i] = inputPixel[i] * m_Scale[i];
          }
        else
          {
          outputPixel[i] = m_NoDataValue;
          }
        }
      outputIt.Set(outputPixel);

      ++inputIt;
      ++outputIt;
      progress.CompletedPixel();
      }
  }

private:
  MaskedScaleImageFilter(const Self &) = delete;
  void operator =(const Self&) = delete;

  InputInternalPixelType    m_NoDataValue;
  InputPixelType            m_Scale;
};
