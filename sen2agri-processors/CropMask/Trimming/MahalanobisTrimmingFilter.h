#ifndef TRIMMING_HXX
#define TRIMMING_HXX

#include "itkImageToImageFilter.h"
#include <list>


template< typename TInputImage, typename TOutputImage >
class MahalanobisTrimmingFilter:
  public itk::ImageToImageFilter< TInputImage, TOutputImage >
{
public:
  /** Standard class typedefs. */
  typedef MahalanobisTrimmingFilter            Self;
  typedef itk::ImageToImageFilter< TInputImage, TOutputImage > Superclass;
  typedef itk::SmartPointer< Self >                            Pointer;
  typedef itk::SmartPointer< const Self >                      ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods).  */
  itkTypeMacro(MahalanobisTrimmingFilter,
               ImageToImageFilter);

  typedef TInputImage                         InputImageType;
  typedef typename InputImageType::Pointer    InputImagePointer;
  typedef typename InputImageType::RegionType InputImageRegionType;
  typedef typename InputImageType::PixelType  InputImagePixelType;
  typedef typename InputImageType::IndexType  IndexType;
  typedef typename InputImageType::SizeType   SizeType;

  typedef TOutputImage                         OutputImageType;
  typedef typename OutputImageType::Pointer    OutputImagePointer;
  typedef typename OutputImageType::RegionType OutputImageRegionType;
  typedef typename OutputImageType::PixelType  OutputImagePixelType;

  typedef std::vector< IndexType > PointsContainerType;

  void PrintSelf(std::ostream & os, itk::Indent indent) const ITK_OVERRIDE;

  /** Add evaluated point. */
  void AddPoint(const IndexType & seed);

  /** Remove all points */
  void ClearPoints();

  /** Set/Get value to replace thresholded pixels */
  itkSetMacro(ReplaceValue, OutputImagePixelType);
  itkGetConstMacro(ReplaceValue, OutputImagePixelType);

  /** Set/Get the value of the parameter Alpha used for the Chi-Squared Distribution */
  itkSetMacro(Alpha, double);
  itkGetConstMacro(Alpha, double);

  /** Set/Get the value of the Number of samples that must be returned.*/
  itkSetMacro(NbSamples, int);
  itkGetConstMacro(NbSamples, int);

  /** Set/Get the value of the Seed or the random number generation.*/
  itkSetMacro(Seed, int);
  itkGetConstMacro(Seed, int);

  /** Set/Get value of the class */
  itkSetMacro(Class, short);
  itkGetConstMacro(Class, short);


protected:
  MahalanobisTrimmingFilter();

  // Override since the filter needs all the data for the algorithm
  void GenerateInputRequestedRegion() ITK_OVERRIDE;

  // Override since the filter produces the entire dataset
  void EnlargeOutputRequestedRegion(itk::DataObject *output) ITK_OVERRIDE;

  void GenerateData() ITK_OVERRIDE;

private:
  MahalanobisTrimmingFilter(const Self &); //purposely not
                                                      // implemented
  void operator=(const Self &);                       //purposely not

  // implemented

  PointsContainerType  m_Points;
  double               m_Alpha;
  OutputImagePixelType m_ReplaceValue;
  short                m_Class;
  int                  m_NbSamples;
  int                  m_Seed;
};

#ifndef ITK_MANUAL_INSTANTIATION
#include "MahalanobisTrimmingFilter.hxx"
#endif


#endif // TRIMMING_HXX

