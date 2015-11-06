#ifndef TEMPORALRESAMPLING_HXX
#define TEMPORALRESAMPLING_HXX



#include "MahalanobisTrimmingFilterThreaded.h"
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
MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
::MahalanobisTrimmingFilterThreaded()
{
    this->SetNumberOfRequiredInputs(1);
  m_Alpha = 0.01;
  m_NbSamples = 0;
  m_Seed = 0;
  m_Class = 0;
  m_Done = false;
  m_Points.clear();
  m_Pixels.clear();
  m_ReplaceValue = itk::NumericTraits< OutputImagePixelType >::OneValue();
  m_Counts = std::vector<int>(this->GetNumberOfThreads(), 0);
}

/**
 * Standard PrintSelf method.
 */
template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
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
MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
::AddPoint(const IndexType & point)
{
  m_Points.push_back(point);
  m_Pixels.push_back(static_cast<InputImagePixelType>(0));
  this->Modified();
}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
::ClearPoints()
{
  if ( this->m_Points.size() > 0 )
    {
    this->m_Points.clear();
    this->m_Pixels.clear();
    this->Modified();
    }
}

//template< typename TInputImage, typename TOutputImage >
//void
//MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
//::GenerateInputRequestedRegion()
//{
//  Superclass::GenerateInputRequestedRegion();
//  if ( this->GetInput() )
//    {
//    InputImagePointer input =
//      const_cast< TInputImage * >( this->GetInput() );
//    input->SetRequestedRegionToLargestPossibleRegion();
//    }
//}


//template< typename TInputImage, typename TOutputImage >
//void
//MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
//::EnlargeOutputRequestedRegion(itk::DataObject *output)
//{
//  Superclass::EnlargeOutputRequestedRegion(output);
//  output->SetRequestedRegionToLargestPossibleRegion();
//}

//template< typename TInputImage, typename TOutputImage >
//void
//MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
//::AllocateOutputs()
//{
//    //this->GetOutput()->SetRequestedRegionToLargestPossibleRegion();
//    //Superclass::AllocateOutputs();
//}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
::BeforeThreadedGenerateData()
{
    //m_Pixels = PixelsContainerType(m_Points.size(), static_cast<InputImagePixelType>(0));
    //this->GetOutput()->SetRequestedRegionToLargestPossibleRegion();

}

//template< typename TInputImage, typename TOutputImage >
//void
//MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
//::GenerateOutputInformation()
//{
//    Superclass::GenerateOutputInformation();
//    this->GetOutput()->SetNumberOfComponentsPerPixel(1);
//}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
::ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId )
{
    if (m_Done) {
        return;
    }

    typedef itk::ImageRegionConstIterator<TInputImage> ImageRegionConstIteratorType;

    ImageRegionConstIteratorType inputImage = ImageRegionConstIteratorType(this->GetInput(), outputRegionForThread);

    typename PointsContainerType::iterator si = m_Points.begin();
    typename PointsContainerType::iterator li = m_Points.end();

    int counter = 0;
    while (si != li) {
        if (outputRegionForThread.IsInside(*si)) {
            inputImage.SetIndex(*si);
            m_Pixels[counter] = inputImage.Get();
            m_Counts[threadId]++;
        }
        counter++;
        ++si;
    }
}

template< typename TInputImage, typename TOutputImage >
void
MahalanobisTrimmingFilterThreaded< TInputImage, TOutputImage >
::AfterThreadedGenerateData()
{
    if (m_Done) {
        return;
    }
    int size = 0;
    for (int count : m_Counts) {
        size += count;
    }

    if (size != m_Pixels.size()) {
        // Wait for all data to be available
        return;
    }

    itkDebugMacro(<< "Starting processing for class " << m_Class);
    typename Superclass::OutputImagePointer outputImage = this->GetOutput();

    // Zero the output
//    OutputImageRegionType region = outputImage->GetLargestPossibleRegion();
//    outputImage->SetBufferedRegion(region);
//    outputImage->Allocate();
//    outputImage->FillBuffer (itk::NumericTraits< OutputImagePixelType >::ZeroValue());

    const unsigned int dimension = this->GetInput()->GetNumberOfComponentsPerPixel();

    // Compute the chi-squared distribution
    typedef itk::Statistics::ChiSquareDistribution      ChiSquareDistributionType;
    const double chi = ChiSquareDistributionType::InverseCDF(1.0 - m_Alpha, dimension);

    // Compute the statistics of the points
    typedef itk::Statistics::ListSample< InputImagePixelType > SampleType;

    typedef itk::Statistics::MahalanobisDistanceMembershipFunction< InputImagePixelType >  MahalanobisDistanceFunctionType;

    typename MahalanobisDistanceFunctionType::Pointer distance = MahalanobisDistanceFunctionType::New();

    typename SampleType::Pointer sample = SampleType::New();
    sample->SetMeasurementVectorSize( dimension );

    itk::SizeValueType removed_cnt = 1;
    itkDebugMacro(<< "Number of points at the begining: " << m_Points.size());

    while (m_Points.size() > 0 && removed_cnt > 0) {
        // first add the points to the sample
        sample->Clear();

        typename PixelsContainerType::iterator siPix = m_Pixels.begin();
        typename PixelsContainerType::iterator liPix = m_Pixels.end();

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

        typename PointsContainerType::iterator si = m_Points.begin();
        typename PointsContainerType::iterator li = m_Points.end();
        siPix = m_Pixels.begin();
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
        m_Pixels.erase(liPix, m_Pixels.end());
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
    m_Done = true;
    itkDebugMacro(<< "Finished processing class: " << m_Class);

}

#endif // TEMPORALRESAMPLING_HXX

