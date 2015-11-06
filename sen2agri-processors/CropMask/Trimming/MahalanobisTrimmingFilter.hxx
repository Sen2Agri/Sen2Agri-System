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
::MahalanobisTrimmingFilter() : m_Has11(false)
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
  m_Has11 = true;
  if (m_Points.find(11) == m_Points.end()) {
      m_Has11 = false;
  }
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
    outputImage->FillBuffer (static_cast<OutputPixelType>(-1));

    // loop through each class
    for (auto& points : m_Points) {
        OutputPixelType pix;
        if (points.first == 11 || points.first == 20 || (points.first == 10 && !m_Has11)) {
            //Crop
            pix = static_cast<OutputPixelType>(1);
        } else {
            //No Crop
            pix = static_cast<OutputPixelType>(0);
        }
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

