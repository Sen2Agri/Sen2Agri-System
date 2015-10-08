#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX



#include "MahalanobisTrimmingFilter.h"
#include "itkChiSquareDistribution.h"

#include "itkMacro.h"
#include "itkImageRegionIterator.h"
#include "itkVectorMeanImageFunction.h"
#include "itkCovarianceImageFunction.h"
#include "itkBinaryThresholdImageFunction.h"
#include "itkFloodFilledImageFunctionConditionalIterator.h"
#include "itkNumericTraitsRGBPixel.h"
#include "itkProgressReporter.h"

#include "itkListSample.h"
#include "itkCovarianceSampleFilter.h"
#include "itkMahalanobisDistanceMembershipFunction.h"

/**
 * Constructor
 */
template< typename TInputImage, typename TOutputImage >
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::MahalanobisTrimmingFilter()
{
  m_Alpha = 0.01;
  m_Points.clear();
  m_ReplaceValue = itk::NumericTraits< OutputImagePixelType >::OneValue();
}

/**
 * Standard PrintSelf method.
 */
template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::PrintSelf(std::ostream & os, itk::Indent indent) const
{
  this->Superclass::PrintSelf(os, indent);
  os << indent << "Alpha: " << m_Alpha
     << std::endl;
  os << indent << "ReplaceValue: "
     << static_cast< typename itk::NumericTraits< OutputImagePixelType >::PrintType >( m_ReplaceValue )
     << std::endl;
}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::AddPoint(const IndexType & point)
{
  m_Points.push_back(point);
  this->Modified();
}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::ClearPoints()
{
  if ( this->m_Points.size() > 0 )
    {
    this->m_Points.clear();
    this->Modified();
    }
}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();
  if ( this->GetInput() )
    {
    InputImagePointer input =
      const_cast< TInputImage * >( this->GetInput() );
    input->SetRequestedRegionToLargestPossibleRegion();
    }
}


template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::EnlargeOutputRequestedRegion(itk::DataObject *output)
{
  Superclass::EnlargeOutputRequestedRegion(output);
  output->SetRequestedRegionToLargestPossibleRegion();
}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::GenerateData()
{
  typedef typename InputImageType::PixelType InputPixelType;

  typename Superclass::InputImageConstPointer inputImage  = this->GetInput();
  typename Superclass::OutputImagePointer outputImage = this->GetOutput();

  // Zero the output
  OutputImageRegionType region = outputImage->GetRequestedRegion();
  outputImage->SetBufferedRegion(region);
  outputImage->Allocate();
  outputImage->FillBuffer (itk::NumericTraits< OutputImagePixelType >::ZeroValue());

  const unsigned int dimension = inputImage->GetNumberOfComponentsPerPixel();

  // Compute the chi-squared distribution
  typedef itk::Statistics::ChiSquareDistribution      ChiSquareDistributionType;
  const double chi = ChiSquareDistributionType::InverseCDF(1.0 - m_Alpha, dimension);


  // Compute the statistics of the points
  typedef itk::Statistics::ListSample< InputPixelType > SampleType;

  typedef itk::Statistics::MahalanobisDistanceMembershipFunction<
    InputPixelType
    >  MahalanobisDistanceFunctionType;

  typename MahalanobisDistanceFunctionType::Pointer distance = MahalanobisDistanceFunctionType::New();

  typename SampleType::Pointer sample = SampleType::New();
  sample->SetMeasurementVectorSize( dimension );

  itk::SizeValueType removed_cnt = 1;

  while (m_Points.size() > 0 && removed_cnt > 0) {
      // first add the points to the sample
      sample->Clear();

      typename PointsContainerType::iterator si = m_Points.begin();
      typename PointsContainerType::iterator li = m_Points.end();
      removed_cnt = 0;
      while ( si != li )
        {
          sample->PushBack(inputImage->GetPixel(*si));
          ++si;
        }

      // compute the mean and convergence of the sample
      typedef itk::Statistics::CovarianceSampleFilter< SampleType >  CovarianceAlgorithmType;
      typename CovarianceAlgorithmType::Pointer covarianceAlgorithm = CovarianceAlgorithmType::New();
      covarianceAlgorithm->SetInput( sample );
      covarianceAlgorithm->Update();

      distance->SetMean(covarianceAlgorithm->GetMean());
      distance->SetCovariance(covarianceAlgorithm->GetCovarianceMatrix());

      si = m_Points.begin();
      while (si != li) {
          if (distance->Evaluate(inputImage->GetPixel(*si)) > chi) {
              removed_cnt++;
              si = m_Points.erase(si);
          } else {
              ++si;
          }
      }
  }

  // Set in the ouput image only the pixels which passed this filter
  typename PointsContainerType::const_iterator si = m_Points.begin();
  typename PointsContainerType::const_iterator li = m_Points.end();
  while(si != li) {
      outputImage->SetPixel(*si, m_ReplaceValue);
      ++si;
  }


}
#endif // TEMPORALRESAMPLING_HXX

