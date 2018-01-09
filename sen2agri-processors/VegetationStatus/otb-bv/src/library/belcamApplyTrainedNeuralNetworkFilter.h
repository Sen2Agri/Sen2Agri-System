#ifndef _BELCAM_APPLY_TRAINED_NEURAL_NETWORK_
#define _BELCAM_APPLY_TRAINED_NEURAL_NETWORK_ 

#include "otbMacro.h"
#include "itkImageToImageFilter.h"
#include "trainedNeuralNetwork.h"

template <class TI, class TO>
class ITK_EXPORT belcamApplyTrainedNeuralNetworkFilter : public itk::ImageToImageFilter<TI, TO>
{
public:
  // Standard typedefs and macros
  typedef belcamApplyTrainedNeuralNetworkFilter Self;
  typedef itk::ImageToImageFilter<TI, TO> Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  itkNewMacro(Self);
  itkTypeMacro(belcamApplyTrainedNeuralNetworkFilter, itk::ImageToImageFilter);

  void setParams(trained_network &n) { m_net = n; }
  void setScale(double scale) { m_scale = scale; }

protected:
  belcamApplyTrainedNeuralNetworkFilter() {}
  virtual ~belcamApplyTrainedNeuralNetworkFilter() {}
  void ThreadedGenerateData(const typename TO::RegionType & outputRegionForThread, itk::ThreadIdType threadId);
  void GenerateOutputInformation();

private:
  // copy operators purposely not implemented
  belcamApplyTrainedNeuralNetworkFilter(const Self &);
  void operator=(const Self&);
  trained_network m_net;
  double m_scale;

};

#  ifndef ITK_MANUAL_INSTANTIATION
#    include "belcamApplyTrainedNeuralNetworkFilter.txx"
#  endif

#endif
