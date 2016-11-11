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
#ifndef __otbMultiModelImageClassificationFilter_h
#define __otbMultiModelImageClassificationFilter_h

#include "itkImageToImageFilter.h"
#include "otbMachineLearningModel.h"

namespace otb
{
/** \class MultiModelImageClassificationFilter
 *  \brief This filter performs the classification of a VectorImage using a Model.
 *
 *  This filter is streamed and threaded, allowing to classify huge images
 *  while fully using several core.
 *
 * \sa Classifier
 * \ingroup Streamed
 * \ingroup Threaded
 *
 * \ingroup OTBSupervised
 */
template <class TInputImage, class TOutputImage, class TMaskImage = TOutputImage>
class ITK_EXPORT MultiModelImageClassificationFilter
  : public itk::ImageToImageFilter<TInputImage, TOutputImage>
{
public:
  /** Standard typedefs */
  typedef MultiModelImageClassificationFilter                Self;
  typedef itk::ImageToImageFilter<TInputImage, TOutputImage> Superclass;
  typedef itk::SmartPointer<Self>                            Pointer;
  typedef itk::SmartPointer<const Self>                      ConstPointer;

  /** Type macro */
  itkNewMacro(Self)

  /** Creation through object factory macro */
  itkTypeMacro(MultiModelImageClassificationFilter, ImageToImageFilter)

  typedef TInputImage                                InputImageType;
  typedef typename InputImageType::ConstPointer      InputImageConstPointerType;
  typedef typename InputImageType::InternalPixelType ValueType;

  typedef TMaskImage                                 MaskImageType;
  typedef typename MaskImageType::ConstPointer       MaskImageConstPointerType;
  typedef typename MaskImageType::Pointer            MaskImagePointerType;

  typedef TOutputImage                               OutputImageType;
  typedef typename OutputImageType::Pointer          OutputImagePointerType;
  typedef typename OutputImageType::RegionType       OutputImageRegionType;
  typedef typename OutputImageType::PixelType        LabelType;

  typedef MachineLearningModel<ValueType, LabelType> ModelType;
  typedef typename ModelType::Pointer                ModelPointerType;

  typedef typename otb::ObjectList<ModelType>        ModelListType;
  typedef typename ModelListType::Pointer            ModelListPointerType;

  /** Set/Get the model */
  itkSetObjectMacro(Models, ModelListType)
  itkGetObjectMacro(Models, ModelListType)

  /** Set/Get the default label */
  itkSetMacro(DefaultLabel, LabelType)
  itkGetMacro(DefaultLabel, LabelType)

  itkSetMacro(UseModelMask, bool)
  itkGetMacro(UseModelMask, bool)

  void SetModelMap(std::vector<uint8_t> modelMap)
  {
      m_ModelMap = std::move(modelMap);
  }

  const std::vector<uint8_t> & GetModelMap() const
  {
      return m_ModelMap;
  }

  /**
   * If set, the mask pixels specify which input model to use.
   * All pixels with a value greater than 0 in the mask will be classified.
   * \param mask The input mask.
   */
  void SetModelMask(const MaskImageType * mask);

  /**
   * Get the input mask.
   * \return The mask.
   */
  const MaskImageType * GetModelMask(void);

protected:
  /** Constructor */
  MultiModelImageClassificationFilter();
  /** Destructor */
  virtual ~MultiModelImageClassificationFilter() {}

  /** Threaded generate data */
  virtual void ThreadedGenerateData(const OutputImageRegionType& outputRegionForThread, itk::ThreadIdType threadId);
  /** Before threaded generate data */
  virtual void BeforeThreadedGenerateData();
  /**PrintSelf method */
  virtual void PrintSelf(std::ostream& os, itk::Indent indent) const;

private:
  MultiModelImageClassificationFilter(const Self &); //purposely not implemented
  void operator =(const Self&); //purposely not implemented

  /** The model used for classification */
  ModelListPointerType m_Models;
  /** Default label for invalid pixels (when a mask with a pixel of 0) */
  LabelType m_DefaultLabel;

  bool m_UseModelMask;
  std::vector<uint8_t> m_ModelMap;
};
} // End namespace otb
#ifndef OTB_MANUAL_INSTANTIATION
#include "otbMultiModelImageClassificationFilter.txx"
#endif

#endif
