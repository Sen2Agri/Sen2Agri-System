/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __otbStreamingClassStatisticsVectorImageFilter_txx
#define __otbStreamingClassStatisticsVectorImageFilter_txx
#include "otbStreamingClassStatisticsVectorImageFilter.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkProgressReporter.h"
#include "otbMacro.h"

namespace otb
{

template<class TInputImage, class TClassImage, class TPrecision>
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::PersistentStreamingClassStatisticsVectorImageFilter()
 : //m_EnableMinMax(true),
   m_EnableFirstOrderStats(true),
   m_EnableSecondOrderStats(true),
   m_UseUnbiasedEstimator(true),
   m_IgnoreInfiniteValues(true),
   m_IgnoreUserDefinedValue(false),
   m_UserIgnoredValue(itk::NumericTraits<InternalPixelType>::Zero),
   m_ClassPixelCount(this->GetNumberOfThreads())
{
    this->SetNumberOfIndexedInputs(2);
  // first output is a copy of the image, DataObject created by
  // superclass

  // allocate the data objects for the outputs which are
  // just decorators around vector/matrix types
  for (unsigned int i = 1; i < 10; ++i)
    {
    this->itk::ProcessObject::SetNthOutput(i, this->MakeOutput(i).GetPointer());
    }
  // Initiate ignored pixel counters
  // TODO
//  m_ClassPixelCount(this->GetNumberOfThreads())
}

template<class TInputImage, class TClassImage, class TPrecision>
itk::DataObject::Pointer
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::MakeOutput(DataObjectPointerArraySizeType output)
{
  switch (output)
    {
    case 0:
      return static_cast<itk::DataObject*>(TInputImage::New().GetPointer());
      break;
    case 1:
    case 2:
      // min/max
      return static_cast<itk::DataObject*>(PixelObjectType::New().GetPointer());
      break;
    case 3:
    case 4:
      // mean / sum
      return static_cast<itk::DataObject*>(RealPixelMapObjectType::New().GetPointer());
      break;
    case 5:
    case 6:
      // covariance / correlation
      return static_cast<itk::DataObject*>(MatrixMapObjectType::New().GetPointer());
      break;
    case 7:
    case 8:
    case 9:
      // component mean, component covariance, component correlation
      return static_cast<itk::DataObject*>(RealObjectType::New().GetPointer());
      break;
    default:
      // might as well make an image
      return static_cast<itk::DataObject*>(TInputImage::New().GetPointer());
      break;
    }
}

//template<class TInputImage, class TClassImage, class TPrecision>
//typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::PixelObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetMinimumOutput()
//{
//  return static_cast<PixelObjectType*>(this->itk::ProcessObject::GetOutput(1));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//const typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::PixelObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetMinimumOutput() const
//{
//  return static_cast<const PixelObjectType*>(this->itk::ProcessObject::GetOutput(1));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::PixelObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetMaximumOutput()
//{
//  return static_cast<PixelObjectType*>(this->itk::ProcessObject::GetOutput(2));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//const typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::PixelObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetMaximumOutput() const
//{
//  return static_cast<const PixelObjectType*>(this->itk::ProcessObject::GetOutput(2));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetComponentMeanOutput()
//{
//  return static_cast<RealObjectType*>(this->itk::ProcessObject::GetOutput(7));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//const typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetComponentMeanOutput() const
//{
//  return static_cast<const RealObjectType*>(this->itk::ProcessObject::GetOutput(7));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetComponentCorrelationOutput()
//{
//  return static_cast<RealObjectType*>(this->itk::ProcessObject::GetOutput(8));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//const typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetComponentCorrelationOutput() const
//{
//  return static_cast<const RealObjectType*>(this->itk::ProcessObject::GetOutput(8));
//}


//template<class TInputImage, class TClassImage, class TPrecision>
//typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetComponentCovarianceOutput()
//{
//  return static_cast<RealObjectType*>(this->itk::ProcessObject::GetOutput(9));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//const typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetComponentCovarianceOutput() const
//{
//  return static_cast<const RealObjectType*>(this->itk::ProcessObject::GetOutput(9));
//}

template<class TInputImage, class TClassImage, class TPrecision>
typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealPixelMapObjectType*
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::GetMeanOutput()
{
  return static_cast<RealPixelMapObjectType*>(this->itk::ProcessObject::GetOutput(3));
}

template<class TInputImage, class TClassImage, class TPrecision>
const typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealPixelMapObjectType*
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::GetMeanOutput() const
{
  return static_cast<const RealPixelMapObjectType*>(this->itk::ProcessObject::GetOutput(3));
}

//template<class TInputImage, class TClassImage, class TPrecision>
//typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealPixelObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetSumOutput()
//{
//  return static_cast<RealPixelObjectType*>(this->itk::ProcessObject::GetOutput(4));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//const typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::RealPixelObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetSumOutput() const
//{
//  return static_cast<const RealPixelObjectType*>(this->itk::ProcessObject::GetOutput(4));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::MatrixObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetCorrelationOutput()
//{
//  return static_cast<MatrixObjectType*>(this->itk::ProcessObject::GetOutput(5));
//}

//template<class TInputImage, class TClassImage, class TPrecision>
//const typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::MatrixObjectType*
//PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
//::GetCorrelationOutput() const
//{
//  return static_cast<const MatrixObjectType*>(this->itk::ProcessObject::GetOutput(5));
//}

template<class TInputImage, class TClassImage, class TPrecision>
typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::MatrixMapObjectType*
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::GetCovarianceOutput()
{
  return static_cast<MatrixMapObjectType*>(this->itk::ProcessObject::GetOutput(6));
}

template<class TInputImage, class TClassImage, class TPrecision>
const typename PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>::MatrixMapObjectType*
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::GetCovarianceOutput() const
{
  return static_cast<const MatrixMapObjectType*>(this->itk::ProcessObject::GetOutput(6));
}

template<class TInputImage, class TClassImage, class TPrecision>
void
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
  if (this->GetInput())
    {
    this->GetOutput()->CopyInformation(this->GetInput());
    this->GetOutput()->SetLargestPossibleRegion(this->GetInput()->GetLargestPossibleRegion());

    if (this->GetOutput()->GetRequestedRegion().GetNumberOfPixels() == 0)
      {
      this->GetOutput()->SetRequestedRegion(this->GetOutput()->GetLargestPossibleRegion());
      }
    }
}

template<class TInputImage, class TClassImage, class TPrecision>
void
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::AllocateOutputs()
{
  // This is commented to prevent the streaming of the whole image for the first stream strip
  // It shall not cause any problem because the output image of this filter is not intended to be used.
  //InputImagePointer image = const_cast< TInputImage * >( this->GetInput() );
  //this->GraftOutput( image );
  // Nothing that needs to be allocated for the remaining outputs
}

template<class TInputImage, class TClassImage, class TPrecision>
void
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::Reset()
{
  TInputImage * inputPtr = const_cast<TInputImage *>(this->GetInput());
  inputPtr->UpdateOutputInformation();

  TClassImage * classInputPtr = const_cast<TClassImage *>(this->GetClassInput());
  classInputPtr->UpdateOutputInformation();

  unsigned int numberOfThreads = this->GetNumberOfThreads();

//  if (m_EnableMinMax)
//    {
//    PixelType tempPixel;
//    tempPixel.SetSize(numberOfComponent);

//    tempPixel.Fill(itk::NumericTraits<InternalPixelType>::max());
//    this->GetMinimumOutput()->Set(tempPixel);

//    tempPixel.Fill(itk::NumericTraits<InternalPixelType>::NonpositiveMin());
//    this->GetMaximumOutput()->Set(tempPixel);

//    PixelType tempTemporiesPixel;
//    tempTemporiesPixel.SetSize(numberOfComponent);
//    tempTemporiesPixel.Fill(itk::NumericTraits<InternalPixelType>::max());
//    m_ThreadMin = std::vector<PixelType>(numberOfThreads, tempTemporiesPixel);

//    tempTemporiesPixel.Fill(itk::NumericTraits<InternalPixelType>::NonpositiveMin());
//    m_ThreadMax = std::vector<PixelType>(numberOfThreads, tempTemporiesPixel);
//    }

  if (m_EnableSecondOrderStats)
    {
    m_EnableFirstOrderStats = true;
    }

  if (m_EnableFirstOrderStats)
    {
    this->GetMeanOutput()->Set(RealPixelMapType());
//    this->GetSumOutput()->Set(zeroRealPixel);

    m_ThreadFirstOrderAccumulators.resize(numberOfThreads);
    for (auto &e : m_ThreadFirstOrderAccumulators)
        e.clear();

//    RealType zeroReal = itk::NumericTraits<RealType>::ZeroValue();
//    m_ThreadFirstOrderComponentAccumulators.resize(numberOfThreads);
//    std::fill(m_ThreadFirstOrderComponentAccumulators.begin(), m_ThreadFirstOrderComponentAccumulators.end(), zeroReal);

    }

  if (m_EnableSecondOrderStats)
    {
    this->GetCovarianceOutput()->Set(MatrixMapType());
//    this->GetCorrelationOutput()->Set(zeroMatrix);

    m_ThreadSecondOrderAccumulators.resize(numberOfThreads);
//    std::fill(m_ThreadSecondOrderAccumulators.begin(), m_ThreadSecondOrderAccumulators.end(), zeroMatrix);

//    RealType zeroReal = itk::NumericTraits<RealType>::ZeroValue();
//    m_ThreadSecondOrderComponentAccumulators.resize(numberOfThreads);
//    std::fill(m_ThreadSecondOrderComponentAccumulators.begin(), m_ThreadSecondOrderComponentAccumulators.end(), zeroReal);
    }

  m_ClassPixelCount.resize(numberOfThreads);
  std::fill(m_ClassPixelCount.begin(), m_ClassPixelCount.end(), MapType<size_t>{});
}

template<class TInputImage, class TClassImage, class TPrecision>
void
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::Synthetize()
{
  TInputImage * inputPtr = const_cast<TInputImage *>(this->GetInput());
  const unsigned int numberOfComponents = inputPtr->GetNumberOfComponentsPerPixel();

//  PixelType minimum;
//  minimum.SetSize(numberOfComponent);
//  minimum.Fill(itk::NumericTraits<InternalPixelType>::max());
//  PixelType maximum;
//  maximum.SetSize(numberOfComponent);
//  maximum.Fill(itk::NumericTraits<InternalPixelType>::NonpositiveMin());

  RealPixelMapType streamFirstOrderAccumulator;
  MatrixMapType streamSecondOrderAccumulator;

//  RealType streamFirstOrderComponentAccumulator = itk::NumericTraits<RealType>::Zero;
//  RealType streamSecondOrderComponentAccumulator = itk::NumericTraits<RealType>::Zero;

  MapType<size_t> classPixelCount;

  // Accumulate results from all threads
  const itk::ThreadIdType numberOfThreads = this->GetNumberOfThreads();
  for (itk::ThreadIdType threadId = 0; threadId < numberOfThreads; ++threadId)
    {
//    if (m_EnableMinMax)
//      {
//      const PixelType& threadMin  = m_ThreadMin [threadId];
//      const PixelType& threadMax  = m_ThreadMax [threadId];

//      for (unsigned int j = 0; j < numberOfComponent; ++j)
//        {
//        if (threadMin[j] < minimum[j])
//          {
//          minimum[j] = threadMin[j];
//          }
//        if (threadMax[j] > maximum[j])
//          {
//          maximum[j] = threadMax[j];
//          }
//        }
//      }

    if (m_EnableFirstOrderStats)
      {
        for (const auto &p : m_ThreadFirstOrderAccumulators[threadId])
        {
            auto it = streamFirstOrderAccumulator.find(p.first);
            if (it == streamFirstOrderAccumulator.end())
            {
                RealPixelType zeroRealPixel;
                zeroRealPixel.SetSize(numberOfComponents);
                zeroRealPixel.Fill(itk::NumericTraits<PrecisionType>::Zero);

                it = streamFirstOrderAccumulator.emplace(std::make_pair(p.first, zeroRealPixel)).first;
            }

            it->second += p.second;
        }
//      streamFirstOrderComponentAccumulator += m_ThreadFirstOrderComponentAccumulators[threadId];
      }

    if (m_EnableSecondOrderStats)
      {
        for (const auto &p : m_ThreadSecondOrderAccumulators[threadId])
        {

          auto it = streamSecondOrderAccumulator.find(p.first);
          if (it == streamSecondOrderAccumulator.end())
          {
              MatrixType zeroMatrix;
              zeroMatrix.SetSize(numberOfComponents, numberOfComponents);
              zeroMatrix.Fill(itk::NumericTraits<PrecisionType>::Zero);

              it = streamSecondOrderAccumulator.emplace(std::make_pair(p.first, zeroMatrix)).first;
          }

          it->second += p.second;
        }
//      streamSecondOrderComponentAccumulator += m_ThreadSecondOrderComponentAccumulators[threadId];
      }
    for (const auto &p : m_ClassPixelCount[threadId])
        classPixelCount[p.first] += p.second;
    }

    for (auto &p : classPixelCount) {
        if (!p.second)
            itkExceptionMacro("Statistics cannot be calculated with zero relevant pixels.");
    }
//  // Final calculations
//  if (m_EnableMinMax)
//    {
//    this->GetMinimumOutput()->Set(minimum);
//    this->GetMaximumOutput()->Set(maximum);
//    }

  RealPixelMapType mean;
  if (m_EnableFirstOrderStats)
    {
//    this->GetComponentMeanOutput()->Set(streamFirstOrderComponentAccumulator / (nbRelevantPixels * numberOfComponent));
    for (const auto &p : streamFirstOrderAccumulator) {
        auto classValue = p.first;
        auto classRelevantPixels = classPixelCount[classValue];

        mean[p.first] = p.second / classRelevantPixels;
    }
    this->GetMeanOutput()->Set(mean);
//    this->GetSumOutput()->Set(streamFirstOrderAccumulator);
    }

  MatrixMapType cov;
  if (m_EnableSecondOrderStats)
    {
    for (auto &p : streamSecondOrderAccumulator)
    {
        auto classValue = p.first;
        auto classRelevantPixels = classPixelCount[classValue];

        MatrixType cor = p.second / classRelevantPixels;
    //    this->GetCorrelationOutput()->Set(cor);

        const auto& classMean = mean[classValue];

        double regul = 1.0;
    //    double regulComponent = 1.0;

        if( m_UseUnbiasedEstimator && classRelevantPixels>1 )
          {
          regul =
           static_cast< double >( classRelevantPixels ) /
           ( static_cast< double >( classRelevantPixels ) - 1.0 );
          }

        //    if( m_UseUnbiasedEstimator && (nbRelevantPixels * numberOfComponent) > 1 )
        //      {
        //      regulComponent =
        //        static_cast< double >(nbRelevantPixels * numberOfComponent) /
        //       ( static_cast< double >(nbRelevantPixels * numberOfComponent) - 1.0 );
        //      }

        auto classCov  = cor;
        for (unsigned int r = 0; r < numberOfComponents; ++r)
          {
          for (unsigned int c = 0; c < numberOfComponents; ++c)
            {
            classCov(r, c) = regul * (classCov(r, c) - classMean[r] * classMean[c]);
            }
          }

        cov[classValue] = std::move(classCov);

    //    this->GetComponentMeanOutput()->Set(streamFirstOrderComponentAccumulator / (nbRelevantPixels * numberOfComponent));
    //    this->GetComponentCorrelationOutput()->Set(streamSecondOrderComponentAccumulator / (nbRelevantPixels * numberOfComponent));
    //    this->GetComponentCovarianceOutput()->Set(
    //        regulComponent * (this->GetComponentCorrelation()
    //           - (this->GetComponentMean() * this->GetComponentMean())));
    }

    this->GetCovarianceOutput()->Set(cov);
  }
}

template<class TInputImage, class TClassImage, class TPrecision>
void
PersistentStreamingClassStatisticsVectorImageFilter<TInputImage, TClassImage, TPrecision>
::ThreadedGenerateData(const RegionType& outputRegionForThread, itk::ThreadIdType threadId)
 {
  // Support progress methods/callbacks
  itk::ProgressReporter progress(this, threadId, outputRegionForThread.GetNumberOfPixels());

  // Grab the input
  InputImagePointer inputPtr = const_cast<TInputImage *>(this->GetInput());
  ClassImagePointer classInputPtr = this->GetClassInput();

  const unsigned int numberOfComponents = inputPtr->GetNumberOfComponentsPerPixel();

//  PixelType& threadMin  = m_ThreadMin [threadId];
//  PixelType& threadMax  = m_ThreadMax [threadId];


  itk::ImageRegionConstIteratorWithIndex<TInputImage> it(inputPtr, outputRegionForThread);
  itk::ImageRegionConstIteratorWithIndex<TClassImage> itClass(classInputPtr, outputRegionForThread);

  for (it.GoToBegin(), itClass.GoToBegin(); !it.IsAtEnd() && !itClass.IsAtEnd(); ++it, ++itClass, progress.CompletedPixel())
    {
    const PixelType& vectorValue = it.Get();
    const ClassType classValue = itClass.Get();

    float finiteProbe = 0.;
    bool userProbe = m_IgnoreUserDefinedValue;
    for (unsigned int j = 0; j < numberOfComponents; ++j)
      {
      finiteProbe += (float)(vectorValue[j]);
      userProbe = userProbe && (vectorValue[j] == m_UserIgnoredValue);
      }

    if (!m_IgnoreInfiniteValues || vnl_math_isfinite(finiteProbe))
      {
      if (!userProbe)
        {
          m_ClassPixelCount[threadId][classValue] ++;
//        if (m_EnableMinMax)
//          {
//          for (unsigned int j = 0; j < vectorValue.GetSize(); ++j)
//            {
//            if (vectorValue[j] < threadMin[j])
//              {
//              threadMin[j] = vectorValue[j];
//              }
//            if (vectorValue[j] > threadMax[j])
//              {
//              threadMax[j] = vectorValue[j];
//              }
//            }
//          }

        if (m_EnableFirstOrderStats)
          {
            auto it = m_ThreadFirstOrderAccumulators[threadId].find(classValue);
            if (it == m_ThreadFirstOrderAccumulators[threadId].end())
            {
                RealPixelType zeroRealPixel;
                zeroRealPixel.SetSize(numberOfComponents);
                zeroRealPixel.Fill(itk::NumericTraits<PrecisionType>::Zero);

                it = m_ThreadFirstOrderAccumulators[threadId].emplace(std::make_pair(classValue, zeroRealPixel)).first;
            }
            RealPixelType&    threadFirstOrder = it->second;
//          RealType& threadFirstOrderComponent  = m_ThreadFirstOrderComponentAccumulators [threadId];

          threadFirstOrder += vectorValue;

//          for (unsigned int i = 0; i < vectorValue.GetSize(); ++i)
//            {
//            threadFirstOrderComponent += vectorValue[i];
//            }
          }

        if (m_EnableSecondOrderStats)
          {
          auto it = m_ThreadSecondOrderAccumulators[threadId].find(classValue);
          if (it == m_ThreadSecondOrderAccumulators[threadId].end())
          {
              MatrixType zeroMatrix;
              zeroMatrix.SetSize(numberOfComponents, numberOfComponents);
              zeroMatrix.Fill(itk::NumericTraits<PrecisionType>::Zero);

              it = m_ThreadSecondOrderAccumulators[threadId].emplace(std::make_pair(classValue, zeroMatrix)).first;
          }
          MatrixType&    threadSecondOrder = it->second;
//          RealType& threadSecondOrderComponent = m_ThreadSecondOrderComponentAccumulators[threadId];

          for (unsigned int r = 0; r < threadSecondOrder.Rows(); ++r)
            {
            for (unsigned int c = 0; c < threadSecondOrder.Cols(); ++c)
              {
              threadSecondOrder(r, c) += vectorValue[r] * vectorValue[c];
              }
            }
//          threadSecondOrderComponent += vectorValue.GetSquaredNorm();
          }
        }
      }
    }

 }

template <class TImage, class TClassImage, class TPrecision>
void
PersistentStreamingClassStatisticsVectorImageFilter<TImage, TClassImage, TPrecision>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

//  os << indent << "Min: "         << this->GetMinimumOutput()->Get()     << std::endl;
//  os << indent << "Max: "         << this->GetMaximumOutput()->Get()     << std::endl;
  auto nextIndent = indent.GetNextIndent();
  for (const auto &p : this->GetMeanOutput()->Get()) {
      os << nextIndent << "Mean for class " << p.first << ": " << p.second << std::endl;
      os << nextIndent << "Covariance for class " << p.first << ": " << p.second << std::endl;
  }
//  os << indent << "Correlation: " << this->GetCorrelationOutput()->Get() << std::endl;
//  os << indent << "Component Mean: "        << this->GetComponentMeanOutput()->Get()        << std::endl;
//  os << indent << "Component Covariance: "  << this->GetComponentCovarianceOutput()->Get()  << std::endl;
//  os << indent << "Component Correlation: " << this->GetComponentCorrelationOutput()->Get() << std::endl;
  os << indent << "UseUnbiasedEstimator: "  << (this->m_UseUnbiasedEstimator ? "true" : "false")  << std::endl;
}

} // end namespace otb
#endif
