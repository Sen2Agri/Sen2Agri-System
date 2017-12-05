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
#include "itkMacro.h"
/**
 * Constructor
 */
template< typename TInputImage, typename TOutputImage >
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::MahalanobisTrimmingFilter()
{
  m_Points.clear();
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
}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::SetPoints(const IndexMapType & points)
{
  m_Points = points;
}

//template< typename TInputImage, typename TOutputImage >
//void
//MahalanobisTrimmingFilter< TInputImage, TOutputImage >
//::GenerateInputRequestedRegion()
//{
//  Superclass::GenerateInputRequestedRegion();
//  if ( this->GetInput() )
//    {
//    InputImagePointer input =
//      const_cast< TInputImage * >( this->GetInput() );
//    input->SetRequestedRegionToLargestPossibleRegion();size()
//    }
//}


//template< typename TInputImage, typename TOutputImage >
//void
//MahalanobisTrimmingFilter< TInputImage, TOutputImage >
//::EnlargeOutputRequestedRegion(itk::DataObject *output)
//{
//  Superclass::EnlargeOutputRequestedRegion(output);
//  output->SetRequestedRegionToLargestPossibleRegion();
//}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilter< TInputImage, TOutputImage >
::GenerateData()
{
    typename Superclass::OutputImagePointer outputImage = this->GetOutput();
    typedef typename OutputImageType::PixelType OutputPixelType;

    // Zero the output
    OutputImageRegionType region = outputImage->GetRequestedRegion();
    outputImage->SetBufferedRegion(region);
    outputImage->Allocate();
    outputImage->FillBuffer (static_cast<OutputPixelType>(-10000));

    // loop through each class
    for (auto& points : m_Points) {
        OutputPixelType pix = points.first;

        typename IndexVectorType::iterator si = points.second.begin();
        typename IndexVectorType::iterator li = points.second.end();

        while (si != li) {
            if (region.IsInside(*si)) {
                // set the value of the pixel
                outputImage->SetPixel(*si, pix);

                // move it to the end for removal.
                std::swap(*si, *(--li));
            } else {
                si++;
            }
        }
        // remove already placed pixels
        points.second.erase(li, points.second.end());

    }
 }

#endif // TEMPORALRESAMPLING_HXX

