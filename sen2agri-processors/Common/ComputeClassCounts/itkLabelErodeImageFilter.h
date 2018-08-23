#ifndef itkLabelErodeImageFilter_h
#define itkLabelErodeImageFilter_h

#include "itkMorphologyImageFilter.h"

namespace itk
{
template< typename TInputImage, typename TOutputImage, typename TKernel >
class ITK_TEMPLATE_EXPORT LabelErodeImageFilter:
  public MorphologyImageFilter< TInputImage, TOutputImage, TKernel >
{
public:
  /** Standard class typedefs. */
  typedef LabelErodeImageFilter                                       Self;
  typedef MorphologyImageFilter< TInputImage, TOutputImage, TKernel > Superclass;
  typedef SmartPointer< Self >                                        Pointer;
  typedef SmartPointer< const Self >                                  ConstPointer;

  /** Standard New method. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(LabelErodeImageFilter,
               MorphologyImageFilter);

  /** Image related typedefs. */
  typedef TInputImage                                InputImageType;
  typedef TOutputImage                               OutputImageType;
  typedef typename TInputImage::RegionType           RegionType;
  typedef typename TInputImage::SizeType             SizeType;
  typedef typename TInputImage::IndexType            IndexType;
  typedef typename TInputImage::PixelType            PixelType;

  typedef typename InputImageType::PixelType                 InputPixelType;
  typedef typename OutputImageType::PixelType                OutputPixelType;
  typedef typename NumericTraits< InputPixelType >::RealType InputRealType;
  typedef typename InputImageType::OffsetType                OffsetType;
  typedef typename InputImageType::IndexValueType            IndexValueType;

  typedef typename InputImageType::RegionType    InputImageRegionType;
  typedef typename OutputImageType::RegionType   OutputImageRegionType;
  typedef typename InputImageType::SizeType      InputSizeType;
  typedef typename InputImageType::SizeValueType InputSizeValueType;

  /** Kernel (structuring element) iterator. */
  typedef typename Superclass::KernelIteratorType KernelIteratorType;

  /** Neighborhood iterator type. */
  typedef typename Superclass::NeighborhoodIteratorType NeighborhoodIteratorType;

  /** Kernel typedef. */
  typedef typename Superclass::KernelType KernelType;

  /** ImageDimension constants */
  itkStaticConstMacro(InputImageDimension, unsigned int,
                      TInputImage::ImageDimension);
  itkStaticConstMacro(OutputImageDimension, unsigned int,
                      TOutputImage::ImageDimension);
  itkStaticConstMacro(KernelDimension, unsigned int,
                      TKernel::NeighborhoodDimension);

  /** Type of the pixels in the Kernel. */
  typedef typename TKernel::PixelType KernelPixelType;

  /** Set the value used as "background". BackgroundValue is used
   * to fill the removed pixels.
   */
  itkSetMacro(BackgroundValue, OutputPixelType);

  /** Get the value used as "background". BackgroundValue is used
   * to fill the removed pixels.
   */
  itkGetConstMacro(BackgroundValue, OutputPixelType);

#ifdef ITK_USE_CONCEPT_CHECKING
  // Begin concept checking
  itkConceptMacro( InputConvertibleToOutputCheck,
                   ( Concept::Convertible< PixelType, typename TOutputImage::PixelType > ) );
  itkConceptMacro( SameDimensionCheck1,
                   ( Concept::SameDimension< InputImageDimension, OutputImageDimension > ) );
  itkConceptMacro( SameDimensionCheck2,
                   ( Concept::SameDimension< InputImageDimension, KernelDimension > ) );
  itkConceptMacro( InputLessThanComparableCheck,
                   ( Concept::LessThanComparable< PixelType > ) );
  itkConceptMacro( KernelGreaterThanComparableCheck,
                   ( Concept::GreaterThanComparable< KernelPixelType > ) );
  // End concept checking
#endif

protected:
  LabelErodeImageFilter();
  ~LabelErodeImageFilter() {}

  /** Evaluate image neighborhood with kernel to find the new value
   * for the center pixel value.
   *
   * It will return the minimum value of the image pixels whose corresponding
   * element in the structuring element is positive. This version of
   * Evaluate is used for non-boundary pixels. */
  virtual PixelType Evaluate(const NeighborhoodIteratorType & nit,
                             const KernelIteratorType kernelBegin,
                             const KernelIteratorType kernelEnd) ITK_OVERRIDE;

private:
  ITK_DISALLOW_COPY_AND_ASSIGN(LabelErodeImageFilter);

  /** Pixel value for background */
  OutputPixelType m_BackgroundValue;
}; // end of class
} // end namespace itk

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkLabelErodeImageFilter.hxx"
#endif

#endif
