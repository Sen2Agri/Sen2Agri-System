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
#ifndef __otbNeuralNetworkRegressionMachineLearningModel_txx
#define __otbNeuralNetworkRegressionMachineLearningModel_txx

#include "otbNeuralNetworkRegressionMachineLearningModel.h"
#include "otbOpenCVUtils.h"
#include "itkMacro.h" // itkExceptionMacro

namespace otb
{

template<class TInputValue, class TOutputValue>
NeuralNetworkRegressionMachineLearningModel<TInputValue, TOutputValue>::NeuralNetworkRegressionMachineLearningModel() :
  m_ANNModel (new CvANN_MLP),
  m_TrainMethod(CvANN_MLP_TrainParams::RPROP),
  m_ActivateFunction(CvANN_MLP::SIGMOID_SYM),
  m_Alpha(1.),
  m_Beta(1.),
  m_BackPropDWScale(0.1),
  m_BackPropMomentScale(0.1),
  m_RegPropDW0(0.1),
  m_RegPropDWMin(FLT_EPSILON),
  m_TermCriteriaType(CV_TERMCRIT_ITER + CV_TERMCRIT_EPS),
  m_MaxIter(1000),
  m_Epsilon(0.01)
{
}

template<class TInputValue, class TOutputValue>
NeuralNetworkRegressionMachineLearningModel<TInputValue, TOutputValue>::~NeuralNetworkRegressionMachineLearningModel()
{
  delete m_ANNModel;
}


/** Train the machine learning model */
template<class TInputValue, class TOutputValue>
void NeuralNetworkRegressionMachineLearningModel<TInputValue, TOutputValue>::Train()
{
  //Create the neural network
  const unsigned int nbLayers = m_LayerSizes.size();

  if ( nbLayers == 0 )
    itkExceptionMacro(<< "Number of layers in the Neural Network must be >= 3")

  cv::Mat layers = cv::Mat(nbLayers, 1, CV_32SC1);
  for (unsigned int i = 0; i < nbLayers; i++)
    {
    layers.row(i) = m_LayerSizes[i];
    }

  m_ANNModel->create(layers, m_ActivateFunction, m_Alpha, m_Beta);

  //convert listsample to opencv matrix
  cv::Mat samples;
  otb::ListSampleToMat<InputListSampleType>(this->GetInputListSample(), samples);

  cv::Mat matOutputANN;
  otb::ListSampleToMat<TargetListSampleType>(this->GetTargetListSample(), matOutputANN);

  CvANN_MLP_TrainParams params;
  params.train_method = m_TrainMethod;
  params.bp_dw_scale = m_BackPropDWScale;
  params.bp_moment_scale = m_BackPropMomentScale;
  params.rp_dw0 = m_RegPropDW0;
  params.rp_dw_min = m_RegPropDWMin;
  CvTermCriteria term_crit = cvTermCriteria(m_TermCriteriaType, m_MaxIter, m_Epsilon);
  params.term_crit = term_crit;

  //train the Neural network model
  m_ANNModel->train(samples, matOutputANN, cv::Mat(), cv::Mat(), params);
}

template<class TInputValue, class TOutputValue>
typename NeuralNetworkRegressionMachineLearningModel<TInputValue, TOutputValue>::TargetSampleType NeuralNetworkRegressionMachineLearningModel<
    TInputValue, TOutputValue>::Predict(const InputSampleType & input) const
{
  //convert listsample to Mat
  cv::Mat sample;

  otb::SampleToMat<InputSampleType>(input, sample);

  cv::Mat response; //(1, 1, CV_32FC1);
  m_ANNModel->predict(sample, response);

  TargetSampleType target;
  target[0] = response.at<float> (0, 0);
  return target;
}

template<class TInputValue, class TOutputValue>
void NeuralNetworkRegressionMachineLearningModel<TInputValue, TOutputValue>::Save(const std::string & filename,
                                                                        const std::string & name)
{
  const char* lname = "my_nn";
  if ( !name.empty() )
    lname = name.c_str();

  CvFileStorage* fs = 0;
  fs = cvOpenFileStorage(filename.c_str(), 0, CV_STORAGE_WRITE);
  if ( !fs )
    {
    itkExceptionMacro("Could not open the file " << filename << " for writing");
    }

  m_ANNModel->write(fs, lname);

  cvReleaseFileStorage(&fs);
}

template<class TInputValue, class TOutputValue>
void NeuralNetworkRegressionMachineLearningModel<TInputValue, TOutputValue>::Load(const std::string & filename,
                                                                        const std::string & name)
{
  const char* lname = 0;
  if ( !name.empty() )
    lname = name.c_str();

  CvFileStorage* fs = 0;
  CvFileNode* model_node = 0;
  fs = cvOpenFileStorage(filename.c_str(), 0, CV_STORAGE_READ);
  if ( !fs )
    {
    itkExceptionMacro("Could not open the file " << filename << " for reading");
    }

  if( lname )
    model_node = cvGetFileNodeByName(fs, 0, lname);
  else
    {
    CvFileNode* root = cvGetRootFileNode( fs );
    if( root->data.seq->total > 0 )
      model_node = (CvFileNode*)cvGetSeqElem( root->data.seq, 0 );
    }

  m_ANNModel->read(fs, model_node);
  cvReleaseFileStorage(&fs);
}

template<class TInputValue, class TOutputValue>
void NeuralNetworkRegressionMachineLearningModel<TInputValue, TOutputValue>::PrintSelf(std::ostream& os, itk::Indent indent) const
{
  // Call superclass implementation
  Superclass::PrintSelf(os, indent);
}

} //end namespace otb

#endif
