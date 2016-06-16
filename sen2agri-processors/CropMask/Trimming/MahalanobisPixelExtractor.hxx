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
#ifndef __otbStreamingStatisticsVectorImageFilter_txx
#define __otbStreamingStatisticsVectorImageFilter_txx
#include "MahalanobisPixelExtractor.h"

#include "itkImageRegionIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
#include "itkProgressReporter.h"
#include "otbMacro.h"

#include "itkChiSquareDistribution.h"

#include "itkVectorMeanImageFunction.h"
#include "itkCovarianceImageFunction.h"
#include "itkBinaryThresholdImageFunction.h"
#include "itkFloodFilledImageFunctionConditionalIterator.h"
#include "itkNumericTraitsRGBPixel.h"
#include "itkProgressReporter.h"

#include "itkListSample.h"
#include "itkCovarianceSampleFilter.h"
#include "itkMahalanobisDistanceMembershipFunction.h"


template<class TInputImage>
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::PersistentMahalanobisPixelExtractorFilter()
   : m_Alpha(0.01),
     m_NbSamples(0),
     m_Seed(0)
{
  // first output is a copy of the image, DataObject created by
  // superclass

  // allocate the data objects for the outputs which are
  // just decorators around vector/matrix types
  this->itk::ProcessObject::SetNthOutput(1, this->MakeOutput(1).GetPointer());
}

template<class TInputImage>
itk::DataObject::Pointer
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::MakeOutput(DataObjectPointerArraySizeType output)
{
  switch (output)
    {
    case 0:
      return static_cast<itk::DataObject*>(TInputImage::New().GetPointer());
      break;
    case 1:
      return static_cast<itk::DataObject*>(IndexMapObjectType::New().GetPointer());
      break;
    default:
      // might as well make an image
      return static_cast<itk::DataObject*>(TInputImage::New().GetPointer());
      break;
    }
}

template<class TInputImage>
void
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::AddPoint(const IndexType & point, const short cls)
{
    // get the vector that corresponds to the class
    m_Points[cls].emplace_back(point);
    m_Pixels[cls].emplace_back(0);
}

template<class TInputImage>
void
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::ClearPoints()
{
  if ( this->m_Points.size() > 0 )
  {
    this->m_Points.clear();
    this->m_Pixels.clear();
  }
}

template<class TInputImage>
typename PersistentMahalanobisPixelExtractorFilter<TInputImage>::IndexMapObjectType*
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::GetIndecesOutput()
{
  return static_cast<IndexMapObjectType*>(this->itk::ProcessObject::GetOutput(1));
}

template<class TInputImage>
const typename PersistentMahalanobisPixelExtractorFilter<TInputImage>::IndexMapObjectType*
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::GetIndecesOutput() const
{
  return static_cast<const IndexMapObjectType*>(this->itk::ProcessObject::GetOutput(1));
}


template<class TInputImage>
void
PersistentMahalanobisPixelExtractorFilter<TInputImage>
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

template<class TInputImage>
void
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::AllocateOutputs()
{
  // This is commented to prevent the streaming of the whole image for the first stream strip
  // It shall not cause any problem because the output image of this filter is not intended to be used.
  //InputImagePointer image = const_cast< TInputImage * >( this->GetInput() );
  //this->GraftOutput( image );
  // Nothing that needs to be allocated for the remaining outputs
}

template<class TInputImage>
void
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::Reset()
{
  TInputImage * inputPtr = const_cast<TInputImage *>(this->GetInput());
  inputPtr->UpdateOutputInformation();

  //m_Pixels.clear();
  //m_Points.clear();
}

template<class TInputImage>
void
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::Synthetize()
{
    itkDebugMacro(<< "Starting processing");

    const unsigned int dimension = this->GetInput()->GetNumberOfComponentsPerPixel();

    // Compute the chi-squared distribution
    typedef itk::Statistics::ChiSquareDistribution      ChiSquareDistributionType;
    const double chi = ChiSquareDistributionType::InverseCDF(1.0 - m_Alpha, dimension);

    // Compute the statistics of the points
    typedef itk::Statistics::ListSample< PixelType > SampleType;

    typedef itk::Statistics::MahalanobisDistanceMembershipFunction< PixelType >  MahalanobisDistanceFunctionType;

    typename MahalanobisDistanceFunctionType::Pointer distance = MahalanobisDistanceFunctionType::New();

    typename SampleType::Pointer sample = SampleType::New();
    sample->SetMeasurementVectorSize( dimension );

    // initialise the random number generator
    std::srand(m_Seed);
    for (auto& points : m_Points) {
        itk::SizeValueType removed_cnt = 1;
        itkDebugMacro(<< "Number of points at the begining: " << points.second.size());

        while (points.second.size() > 0 && removed_cnt > 0) {
            // first add the points to the sample
            sample->Clear();

            PixelVectorType &pixels = m_Pixels[points.first];

            typename PixelVectorType::iterator siPix = pixels.begin();
            typename PixelVectorType::iterator liPix = pixels.end();

            removed_cnt = 0;
            while ( siPix != liPix )
            {
                sample->PushBack(*siPix);
                 ++siPix;
            }

            // compute the mean and convergence of the sample
            typedef itk::Statistics::CovarianceSampleFilter< SampleType >  CovarianceAlgorithmType;
            typename CovarianceAlgorithmType::Pointer covarianceAlgorithm = CovarianceAlgorithmType::New();
            covarianceAlgorithm->SetInput( sample );
            covarianceAlgorithm->Update();

            distance->SetMean(covarianceAlgorithm->GetMean());
            distance->SetCovariance(covarianceAlgorithm->GetCovarianceMatrix());

            typename IndexVectorType::iterator si = points.second.begin();
            typename IndexVectorType::iterator li = points.second.end();
            siPix = pixels.begin();
            while (siPix != liPix) {
                if (distance->Evaluate(*siPix) > chi) {
                    removed_cnt++;
                    // move to end
                    std::swap(*si, *(--li));
                    std::swap(*siPix, *(--liPix));
                    //si = m_Points.erase(si);
                } else {
                    ++si;
                    ++siPix;
                }
            }
            // remove the last elements
            pixels.erase(liPix, pixels.end());
            points.second.erase(li, points.second.end());
        }
        itkDebugMacro(<< "Number of points at the end: " << m_Points.size());

        // Reduce the number of pixels in oder to reduce the time of clasification
        if (m_NbSamples > 0 && points.second.size() > m_NbSamples) {

            typename IndexVectorType::iterator st = points.second.begin();
            int pixSel = 0;
            int size = points.second.size();
            while (pixSel < m_NbSamples) {
                typename IndexVectorType::iterator r = st;
                float rnd = (float)std::rand() / (float)RAND_MAX;
                std::advance(r, (int)(rnd * (float)size));
                std::swap(*st, *r);
                ++st;
                --size;
                ++pixSel;
            }

            // Remove unselected pixels
            points.second.erase(st, points.second.end());
        }
    }



    this->GetIndecesOutput()->Set(m_Points);
}

template<class TInputImage>
void
PersistentMahalanobisPixelExtractorFilter<TInputImage>
::ThreadedGenerateData(const RegionType& outputRegionForThread, itk::ThreadIdType threadId)
 {
    typedef itk::ImageRegionConstIterator<TInputImage> ImageRegionConstIteratorType;

    ImageRegionConstIteratorType inputImage = ImageRegionConstIteratorType(this->GetInput(), outputRegionForThread);

    for (auto& points : m_Points) {
        auto& pixels = m_Pixels[points.first];

        typename IndexVectorType::iterator si = points.second.begin();
        typename IndexVectorType::iterator li = points.second.end();

        int counter = 0;
        while (si != li) {
            if (outputRegionForThread.IsInside(*si)) {
                inputImage.SetIndex(*si);
                pixels[counter] = inputImage.Get();
            }
            counter++;
            ++si;
        }

    }
 }

template <class TImage>
void
PersistentMahalanobisPixelExtractorFilter<TImage>
::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  Superclass::PrintSelf(os, indent);

//  os << indent << "Min: "         << this->GetMinimumOutput()->Get()     << std::endl;
//  os << indent << "Max: "         << this->GetMaximumOutput()->Get()     << std::endl;
//  os << indent << "Mean: "        << this->GetMeanOutput()->Get()        << std::endl;
//  os << indent << "Covariance: "  << this->GetCovarianceOutput()->Get()  << std::endl;
//  os << indent << "Correlation: " << this->GetCorrelationOutput()->Get() << std::endl;
//  os << indent << "Component Mean: "        << this->GetComponentMeanOutput()->Get()        << std::endl;
//  os << indent << "Component Covariance: "  << this->GetComponentCovarianceOutput()->Get()  << std::endl;
//  os << indent << "Component Correlation: " << this->GetComponentCorrelationOutput()->Get() << std::endl;
//  os << indent << "UseUnbiasedEstimator: "  << (this->m_UseUnbiasedEstimator ? "true" : "false")  << std::endl;
}

#endif
