#include "itkImageRegionIterator.h"
#include "belcamApplyTrainedNeuralNetworkFilter.h"


template <class TI, class TO>
void belcamApplyTrainedNeuralNetworkFilter<TI, TO>
::ThreadedGenerateData(const typename TO::RegionType& outputRegionForThread, itk::ThreadIdType threadId)
{
  // remove the unused parameter warning
  (void)threadId;
  
  unsigned int nbBands = this->GetInput(0)->GetNumberOfComponentsPerPixel();
  unsigned int nbLayers = m_net.layers.size();

  // initialize iterators
  typename TI::RegionType inputRegionForThread;
  this->CallCopyOutputRegionToInputRegion(inputRegionForThread, outputRegionForThread);
  itk::ImageRegionIterator     <TO> oIt(this->GetOutput(), outputRegionForThread);
  itk::ImageRegionConstIterator<TI> iIt(this->GetInput(), inputRegionForThread);

  typename TO::PixelType pixelO(1);

  if(m_net.layers[0].weights[0].size() != nbBands) {
    printf("the number of bands and the number of inputs in txt file have to be identical !! (%d vs %d)\n", nbBands, (int)m_net.layers[0].weights[0].size());
    exit(1);
  }

  // init size of processing variables
  std::vector<std::vector<double> > inputs(nbLayers+1);
  for(int iN = 0; iN < nbLayers; ++iN)
    for(int i = 0; i < m_net.layers[iN].weights[0].size(); ++i)
      inputs[iN].push_back(0.);
  inputs[nbLayers].push_back(0.); // one output

  // loop over pixels
  for(oIt.GoToBegin(), iIt.GoToBegin(); ! oIt.IsAtEnd(); ++oIt, ++iIt)
  {
    // normalisation of inputs
    const typename TI::PixelType &pixelI = iIt.Get();
    for(int iB = 0; iB < nbBands; ++iB)
      inputs[0][iB] = 2.0 * (pixelI[iB] / m_scale - m_net.min_norm_in[iB]) / (m_net.max_norm_in[iB] - m_net.min_norm_in[iB]) - 1.0;

    // loop over layers
    for(int iL = 0; iL < nbLayers; ++iL) {
      trained_network_layer & lay = m_net.layers[iL];

      // loop over neurons (apply layer rules)
      for(int iN = 0; iN < lay.weights.size(); ++iN) {
        inputs[iL+1][iN] = lay.bias[iN];
        for(int iB = 0; iB < lay.weights[iN].size(); ++iB)
          inputs[iL+1][iN] += inputs[iL][iB] * lay.weights[iN][iB];
        inputs[iL+1][iN] = (*lay.func)(inputs[iL+1][iN]);
      }

    }

    // de-normalisation of output
    pixelO[0] = 0.5 * (inputs[nbLayers][0] + 1.0) * (m_net.max_norm_out - m_net.min_norm_out) + m_net.min_norm_out;

    oIt.Set( pixelO );
  }

}

template <class TI, class TO>
void belcamApplyTrainedNeuralNetworkFilter<TI, TO>
::GenerateOutputInformation()
{
  Superclass::GenerateOutputInformation();
  this->GetOutput()->SetNumberOfComponentsPerPixel( 1 );
}



