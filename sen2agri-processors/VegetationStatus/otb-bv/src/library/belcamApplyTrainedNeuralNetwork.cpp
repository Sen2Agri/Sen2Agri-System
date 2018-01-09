#include <cstdlib> /* putenv */
#include <vector>
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbVectorImage.h"
#include "belcamApplyTrainedNeuralNetworkFilter.h"

#define SCALAR_INPUT short int
#define SCALAR_OUTPUT float

int main(int argc, char * argv[])
{
  putenv((char*)"GDAL_CACHEMAX=4000");

  // input arguments
  if(argc < 5)
  {
    printf("Create a metric based on a trained network and input reflectances.\n");
    printf("Min four args are mandatory: new output file, network text file, input file (multi bands), NaN value.\n");
    printf("                one opt arg: scaling factor on input (default 1)\n");
    exit(EXIT_FAILURE);
  }
  char outputFilename[200];
  sprintf(outputFilename, "%s?&gdal:co:TILED=YES&gdal:co:INTERLEAVE=BAND&gdal:co:BIGTIFF=YES&gdal:co:COMPRESS=LZW", argv[1]);
  std::string inputTextFilename = argv[2];
  std::string inputFilename = argv[3];
  float nan = atof(argv[4]);
  double scale = 1.;
  if(argc > 5)
    scale = atof(argv[5]);

  // extract params from formated text file (mustache template)
  trained_network params = bvnet_fill_trained_network(inputTextFilename);

  // define the pipeline
  typedef otb::VectorImage<SCALAR_INPUT, 2> inputImageType;
  typedef otb::VectorImage<SCALAR_OUTPUT, 2> outputImageType;
  typedef belcamApplyTrainedNeuralNetworkFilter<inputImageType, outputImageType> neuronFilter;
  otb::ImageFileReader<inputImageType>::Pointer readerF = otb::ImageFileReader<inputImageType>::New();
  neuronFilter::Pointer neuronF = neuronFilter::New();
  otb::ImageFileWriter<outputImageType>::Pointer writerF = otb::ImageFileWriter<outputImageType>::New();

  readerF->SetFileName(inputFilename);
  neuronF->SetInput(readerF->GetOutput() );
  neuronF->setParams( params );
  neuronF->setScale( scale );
  writerF->SetInput( neuronF->GetOutput() );
  writerF->SetFileName( outputFilename );

  // execute the pipeline
  writerF->Update();

}


