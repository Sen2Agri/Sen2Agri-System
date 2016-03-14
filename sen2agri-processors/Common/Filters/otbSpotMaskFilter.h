#ifndef __otbSpotMaskFilter_h
#define __otbSpotMaskFilter_h

#include "itkTernaryFunctorImageFilter.h"
#include "otbVectorImage.h"

#define NODATA          -10000


namespace otb
{


template <typename PixelType>
class SpotMaskFunctor
{
public:
    SpotMaskFunctor() {}

    PixelType operator()(const PixelType &maskValidity, const PixelType &maskSaturation, const PixelType &maskClouds) const
    {
        PixelType result(1);

        result[0] = (((static_cast<unsigned short>(maskValidity[0]) & 0x01) |
                       static_cast<unsigned short>(maskSaturation[0]) |
                       static_cast<unsigned short>(maskClouds[0])) == 0) ? 0 : 1;

        return result;
    }

    bool operator!=(const SpotMaskFunctor) const
    {
        return false ;
    }

    bool operator==(const SpotMaskFunctor) const
    {
        return true;
    }
};


/** \class SpotMaskFilter
 * \brief This filter builds the mask for a spot product.
 *
 * \ingroup OTBImageManipulation
 */
template<class TImage>
class ITK_EXPORT SpotMaskFilter
  : public itk::TernaryFunctorImageFilter<TImage,TImage, TImage, TImage, SpotMaskFunctor<typename TImage::PixelType> >
{
public:
  /** Standard class typedefs. */
  typedef SpotMaskFilter                       Self;
  typedef itk::TernaryFunctorImageFilter<TImage,TImage, TImage, TImage, SpotMaskFunctor<typename TImage::PixelType> > Superclass;
  typedef itk::SmartPointer<Self>                             Pointer;
  typedef itk::SmartPointer<const Self>                       ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(VectorImageToImagePixelAccessor, TernaryFunctorImageFilter);

  /** Template related typedefs */
  typedef TImage ImageType;

  typedef typename ImageType::Pointer  ImagePointerType;

  typedef typename ImageType::PixelType        ImagePixelType;

  typedef typename ImageType::InternalPixelType InternalPixelType;
  typedef typename ImageType::RegionType        ImageRegionType;


  /** ImageDimension constant */
  itkStaticConstMacro(OutputImageDimension, unsigned int, TImage::ImageDimension);

  /** Set the SPOT validity mask. */
  void SetInputValidityMask(const TImage *validityMask);

  /** Set the SPOT saturation mask. */
  void SetInputSaturationMask(const TImage *saturationMask);

  /** Set the SPOT Cloud mask. */
  void SetInputCloudsMask(const TImage *cloudsMask);


protected:
  /** Constructor. */
  SpotMaskFilter();
  /** Destructor. */
  virtual ~SpotMaskFilter();
  virtual void GenerateOutputInformation();
  /** PrintSelf method */
  void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  SpotMaskFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

};
} // end namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbSpotMaskFilter.txx"
#endif
#endif
