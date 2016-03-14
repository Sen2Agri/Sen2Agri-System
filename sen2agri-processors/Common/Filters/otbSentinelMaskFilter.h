#ifndef __otbSentinelMaskFilter_h
#define __otbSentinelMaskFilter_h

#include "itkBinaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define NODATA          -10000


namespace otb
{


template <typename PixelType>
class SentinelMaskFunctor
{
public:
    SentinelMaskFunctor() {}

    PixelType operator()(const PixelType &maskQuality, const PixelType &maskClouds) const
    {
        PixelType result(1);

        // Band 3 from the Quality mask contains the pixel validity
        // Band 1 from the Quality mask contains the saturation
        result[0] = (((static_cast<unsigned short>(maskQuality[2]) & 0x01) |
                       static_cast<unsigned short>(maskQuality[0]) |
                       static_cast<unsigned short>(maskClouds[0])) == 0) ? 0 : 1;

        return result;
    }

    bool operator!=(const SentinelMaskFunctor) const
    {
        return false ;
    }

    bool operator==(const SentinelMaskFunctor) const
    {
        return true;
    }
};


/** \class SentinelMaskFilter
 * \brief This filter builds the mask for a Sentinel product.
 *
 * \ingroup OTBImageManipulation
 */
template<class TImage>
class ITK_EXPORT SentinelMaskFilter
  : public itk::BinaryFunctorImageFilter<TImage, TImage, TImage, SentinelMaskFunctor<typename TImage::PixelType> >
{
public:
  /** Standard class typedefs. */
  typedef SentinelMaskFilter                       Self;
  typedef itk::BinaryFunctorImageFilter<TImage, TImage, TImage, SentinelMaskFunctor<typename TImage::PixelType> > Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(VectorImageToImagePixelAccessor, BinaryFunctorImageFilter);

  /** Template related typedefs */
  typedef TImage ImageType;

  typedef typename ImageType::Pointer  ImagePointerType;

  typedef typename ImageType::PixelType        ImagePixelType;

  typedef typename ImageType::InternalPixelType InternalPixelType;
  typedef typename ImageType::RegionType        ImageRegionType;


  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TImage::ImageDimension);

  /** Set the Sentinel quality mask (it contains a band for validity and a band for saturation). */
  void SetInputQualityMask(const TImage *qualityMask);

  /** Set the Sentinel Cloud mask. */
  void SetInputCloudsMask(const TImage *cloudsMask);


protected:
  /** Constructor. */
  SentinelMaskFilter();
  /** Destructor. */
  virtual ~SentinelMaskFilter();
  virtual void GenerateOutputInformation();
  /** PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  SentinelMaskFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

};
} // end namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbSentinelMaskFilter.txx"
#endif
#endif
