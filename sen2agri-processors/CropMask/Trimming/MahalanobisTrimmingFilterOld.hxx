/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX



#include "MahalanobisTrimmingFilterOld.h"
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
#include "itkMacro.h"
/**
 * Constructor
 */
template< typename TInputImage, typename TOutputImage >
MahalanobisTrimmingFilterOld< TInputImage, TOutputImage >
::MahalanobisTrimmingFilterOld()
{
  m_Alpha = 0.01;
  m_NbSamples = 0;
  m_Seed = 0;
  m_Class = 0;
  m_Points.clear();
  m_ReplaceValue = itk::NumericTraits< OutputImagePixelType >::OneValue();
}

/**
 * Standard PrintSelf method.
 */
template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilterOld< TInputImage, TOutputImage >
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
MahalanobisTrimmingFilterOld< TInputImage, TOutputImage >
::AddPoint(const IndexType & point)
{
  m_Points.push_back(point);
  this->Modified();
}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilterOld< TInputImage, TOutputImage >
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
MahalanobisTrimmingFilterOld< TInputImage, TOutputImage >
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
MahalanobisTrimmingFilterOld< TInputImage, TOutputImage >
::EnlargeOutputRequestedRegion(itk::DataObject *output)
{
  Superclass::EnlargeOutputRequestedRegion(output);
  output->SetRequestedRegionToLargestPossibleRegion();
}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilterOld< TInputImage, TOutputImage >
::GenerateData()
{
    itkDebugMacro(<< "Starting processing for class " << m_Class);
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
  itkDebugMacro(<< "Number of points at the begining: " << m_Points.size());

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
              // move to end
              std::swap(*si, *(--li));
              //si = m_Points.erase(si);
          } else {
              ++si;
          }
      }
      // remove the last elements
      m_Points.erase(li, m_Points.end());
  }
  itkDebugMacro(<< "Number of points at the end: " << m_Points.size());

  // Reduce the number of pixels in oder to reduce the time of clasification
  if (m_NbSamples > 0 && m_Points.size() > m_NbSamples) {

      typename PointsContainerType::iterator st = m_Points.begin();
      int pixSel = 0;
      int size = m_Points.size();
      // initialise the random number generator
      std::srand(m_Seed);
      while (pixSel < m_NbSamples) {
          typename PointsContainerType::iterator r = st;
          float rnd = (float)std::rand() / (float)RAND_MAX;
          std::advance(r, (int)(rnd * (float)size));
          std::swap(*st, *r);
          ++st;
          --size;
          ++pixSel;
      }

      // Remove unselected pixels
      m_Points.erase(st, m_Points.end());
  }
  itkDebugMacro(<< "Writing raster. ");

  // Set in the ouput image only the pixels which passed this filter
  typename PointsContainerType::const_iterator si = m_Points.begin();
  typename PointsContainerType::const_iterator li = m_Points.end();
  while(si != li) {
      outputImage->SetPixel(*si, m_ReplaceValue);
      ++si;
  }
  itkDebugMacro(<< "Finished processing class: " << m_Class);
}

#endif // TEMPORALRESAMPLING_HXX

