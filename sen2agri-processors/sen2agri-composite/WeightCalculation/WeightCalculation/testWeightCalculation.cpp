#include "weightonaot.h"
#include "cloudmaskbinarization.h"
#include "cloudsinterpolation.h"
#include "gaussianfilter.h"
#include "cloudweightcomputation.h"
#include "totalweightcomputation.h"

std::string inFile ("/media/sf_2_Sen2Agri/8_ProductSamples/2_FromMACCS/S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_CLD_R1.DBL.TIF");
//std::string inTest1 ("/media/sf_2_Sen2Agri/8_ProductSamples/2_FromMACCS/S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_FRE_R1.DBL.TIF");
//std::string inTest2 ("/media/sf_2_Sen2Agri/8_ProductSamples/2_FromMACCS/S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/S2A_OPER_SSC_PDTIMG_L2VALD_15SVD____20091211_SRE_R1.DBL.TIF");

std::string inAotFile10M ("/media/sf_2_Sen2Agri/8_ProductSamples/2_FromMACCS/S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_ATB_R1.DBL.TIF");
std::string inAotFile20M ("/media/sf_2_Sen2Agri/8_ProductSamples/2_FromMACCS/S2A_OPER_SSC_L2VALD_15SVD____20091211.DBL.DIR/S2A_OPER_SSC_PDTANX_L2VALD_15SVD____20091211_ATB_R2.DBL.TIF");

std::string outAOTWeight10M("/media/sf_2_Sen2Agri/8_ProductSamples/1_WeightOnAOT_10M.tiff");
std::string outAOTWeight20M("/media/sf_2_Sen2Agri/8_ProductSamples/1_WeightOnAOT_20M.tiff");

std::string outFileBinarize("/media/sf_2_Sen2Agri/8_ProductSamples/2_Intermediate1_Binarize.tiff");
std::string outFileUndersample240M("/media/sf_2_Sen2Agri/8_ProductSamples/2_Intermediate2_Undersample_240M.tiff");
std::string outFileGaussianSmallCld("/media/sf_2_Sen2Agri/8_ProductSamples/2_Intermediate3_GaussianSmallCld.tiff");
std::string outFileGaussianLargeCld("/media/sf_2_Sen2Agri/8_ProductSamples/2_Intermediate4_GaussianLargeCld.tiff");

std::string outFileOversampleSmallCld10M("/media/sf_2_Sen2Agri/8_ProductSamples/2_Intermediate5_OversampleSmallCld_10M.tiff");
std::string outFileOversampleLargeCld10M("/media/sf_2_Sen2Agri/8_ProductSamples/2_Intermediate6_OversampleLargeCld_10M.tiff");
std::string outFileOversampleSmallCld20M("/media/sf_2_Sen2Agri/8_ProductSamples/2_Intermediate7_OversampleSmallCld_20M.tiff");
std::string outFileOversampleLargeCld20M("/media/sf_2_Sen2Agri/8_ProductSamples/2_Intermediate8_OversampleLargeCld_20M.tiff");

std::string outWeightOnClouds10M("/media/sf_2_Sen2Agri/8_ProductSamples/3_WeightOnClouds_10M.tiff");
std::string outWeightOnClouds20M("/media/sf_2_Sen2Agri/8_ProductSamples/3_WeightOnClouds_20M.tiff");

std::string outTotalWeight10M("/media/sf_2_Sen2Agri/8_ProductSamples/4_WeightTotal_10M.tiff");
std::string outTotalWeight20M("/media/sf_2_Sen2Agri/8_ProductSamples/4_WeightTotal_20M.tiff");

void TestWeightOnAOT();
void TestComputeCloudWeight();
void TestTotalWeight();

int main(int argc, char* argv[])
{
    TestWeightOnAOT();
    TestComputeCloudWeight();
    TestTotalWeight();
}

void TestWeightOnAOT()
{
    WeightOnAOT weightOnAot10M;
    weightOnAot10M.SetInputFileName(inAotFile10M);
    weightOnAot10M.SetOutputFileName(outAOTWeight10M);
    weightOnAot10M.SetBand(2);
    weightOnAot10M.SetAotQuantificationValue(0.005);
    weightOnAot10M.SetAotMaxValue(50);
    weightOnAot10M.SetMinAotWeight(0.33);
    weightOnAot10M.SetMaxAotWeight(1);
    weightOnAot10M.Update();
    weightOnAot10M.WriteToOutputFile();

    WeightOnAOT weightOnAot20M;
    weightOnAot20M.SetInputFileName(inAotFile20M);
    weightOnAot20M.SetOutputFileName(outAOTWeight20M);
    weightOnAot20M.SetBand(2);
    weightOnAot20M.SetAotQuantificationValue(0.005);
    weightOnAot20M.SetAotMaxValue(50);
    weightOnAot20M.SetMinAotWeight(0.33);
    weightOnAot20M.SetMaxAotWeight(1);
    weightOnAot20M.Update();
    weightOnAot20M.WriteToOutputFile();
}

void TestComputeCloudWeight()
{
    int inputCloudMaskResolution = 10;
    int coarseResolution = 240;
    float sigmaSmallCloud = 10.0;
    float sigmaLargeCloud = 100.0;

    CloudMaskBinarization cloudMaskBinarization;
    cloudMaskBinarization.SetInputFileName(inFile);
    cloudMaskBinarization.SetOutputFileName(outFileBinarize);
    cloudMaskBinarization.Update();
    cloudMaskBinarization.WriteToOutputFile();

    // perform undersampling to lower resolution
    CloudsInterpolation underSampler;
    //underSampler.SetInputFileName(inFile);
    //underSampler.SetInputImage(cloudMaskBinarization.GetProducedImage());
    underSampler.SetInputImageReader(cloudMaskBinarization.GetOutputImageSource());
    underSampler.SetInputResolution(inputCloudMaskResolution);
    underSampler.SetOutputResolution(coarseResolution);
    underSampler.SetOutputFileName(outFileUndersample240M);
    // By default, the undersampler is using the Bicubic interpolator with a radius of 2
    //undersampler.SetInterpolator(Interpolator_BCO);
    //undersampler.SetBicubicInterpolatorRadius(2);
    underSampler.Update();
    underSampler.WriteToOutputFile();

    // Compute the DistLargeCloud, Low Res
    GaussianFilter gaussianFilterSmallCloud;
    //gaussianFilterSmallCloud.SetInputImage(underSampler.GetProducedImage());
    gaussianFilterSmallCloud.SetInputImageReader(underSampler.GetOutputImageSource());
    //gaussianFilter1.SetInputFileName(inTest1);
    gaussianFilterSmallCloud.SetSigma(sigmaSmallCloud);
    gaussianFilterSmallCloud.SetOutputFileName(outFileGaussianSmallCld);
    gaussianFilterSmallCloud.Update();
    gaussianFilterSmallCloud.WriteToOutputFile();

    GaussianFilter gaussianFilterLargeCloud;
    //gaussianFilter2.SetInputFileName(inTest2);
    //gaussianFilterLargeCloud.SetInputImage(underSampler.GetProducedImage());
    gaussianFilterLargeCloud.SetInputImageReader(underSampler.GetOutputImageSource());
    gaussianFilterLargeCloud.SetSigma(sigmaLargeCloud);
    gaussianFilterLargeCloud.SetOutputFileName(outFileGaussianLargeCld);
    gaussianFilterLargeCloud.Update();
    gaussianFilterLargeCloud.WriteToOutputFile();

    // Resample at full resolution of 10m and 20m
    for(int res = 10; res < 22; res++) {
        // resample at the current small resolution (10 or 20) the small cloud large resolution image
        CloudsInterpolation overSamplerSmallCloud;
        //overSamplerSmallCloud.SetInputImage(gaussianFilterSmallCloud.GetProducedImage());
        //overSampler1.SetInputFileName(outFileInt2);
        overSamplerSmallCloud.SetInputImageReader(gaussianFilterSmallCloud.GetOutputImageSource());
        overSamplerSmallCloud.SetOutputFileName(res == 10 ? outFileOversampleSmallCld10M : outFileOversampleSmallCld20M);
        overSamplerSmallCloud.SetInputResolution(coarseResolution);
        overSamplerSmallCloud.SetOutputResolution(res);
        overSamplerSmallCloud.SetInterpolator(Interpolator_Linear);
        overSamplerSmallCloud.Update();
        overSamplerSmallCloud.WriteToOutputFile();

        // resample at the current small resolution (10 or 20) the large cloud large resolution image
        CloudsInterpolation overSamplerLargeCloud;
        //overSamplerLargeCloud.SetInputImage(gaussianFilterLargeCloud.GetProducedImage());
        //overSampler2.SetInputFileName(outFileInt3);
        overSamplerLargeCloud.SetInputImageReader(gaussianFilterLargeCloud.GetOutputImageSource());
        overSamplerLargeCloud.SetOutputFileName(res == 10 ? outFileOversampleLargeCld10M : outFileOversampleLargeCld20M);
        overSamplerLargeCloud.SetInputResolution(coarseResolution);
        overSamplerLargeCloud.SetOutputResolution(res);
        overSamplerLargeCloud.SetInterpolator(Interpolator_Linear);
        overSamplerLargeCloud.Update();
        overSamplerLargeCloud.WriteToOutputFile();

        // compute the weight on clouds
        CloudWeightComputation cloudWeightComputation;
        //weightOnClouds.SetInput1(overSamplerSmallCloud.GetProducedImage());
        //weightOnClouds.SetInput2(overSamplerLargeCloud.GetProducedImage());
        cloudWeightComputation.SetInputImageReader1(overSamplerSmallCloud.GetOutputImageSource());
        cloudWeightComputation.SetInputImageReader2(overSamplerLargeCloud.GetOutputImageSource());
        cloudWeightComputation.SetOutputFileName(res == 10 ? outWeightOnClouds10M : outWeightOnClouds20M);
        cloudWeightComputation.Update();
        cloudWeightComputation.WriteToOutputFile();

        res += 10;
    }

}

void TestTotalWeight()
{
    std::string inProdFileName("S2A_BLA");
    float weightSensor = 0.33;
    int l2aDate = 10;
    int l3aDate = 20;
    int halfSynthesis = 50;
    float weightOnDateMin = 0.10;

    TotalWeightComputation totalWeightComputation10M;
    // weight on sensor parameters
    totalWeightComputation10M.SetInputProductName(inProdFileName);
    totalWeightComputation10M.SetWeightOnSensor(weightSensor);
    // weight on date parameters
    totalWeightComputation10M.SetL2ADateAsDays(l2aDate);
    totalWeightComputation10M.SetL3ADateAsDays(l3aDate);
    totalWeightComputation10M.SetHalfSynthesisPeriodAsDays(halfSynthesis);
    totalWeightComputation10M.SetWeightOnDateMin(weightOnDateMin);
    // Weights for AOT and Clouds
    totalWeightComputation10M.SetAotWeightFile(outAOTWeight10M);
    totalWeightComputation10M.SetCloudsWeightFile(outWeightOnClouds10M);
    // The output file name
    totalWeightComputation10M.SetTotalWeightOutputFileName(outTotalWeight10M);
    totalWeightComputation10M.Update();
    totalWeightComputation10M.WriteToOutputFile();

    TotalWeightComputation totalWeightComputation20M;
    // weight on sensor parameters
    totalWeightComputation20M.SetInputProductName(inProdFileName);
    totalWeightComputation20M.SetWeightOnSensor(weightSensor);
    // weight on date parameters
    totalWeightComputation20M.SetL2ADateAsDays(l2aDate);
    totalWeightComputation20M.SetL3ADateAsDays(l3aDate);
    totalWeightComputation20M.SetHalfSynthesisPeriodAsDays(halfSynthesis);
    totalWeightComputation20M.SetWeightOnDateMin(weightOnDateMin);
    // Weights for AOT and Clouds
    totalWeightComputation20M.SetAotWeightFile(outAOTWeight20M);
    totalWeightComputation20M.SetCloudsWeightFile(outWeightOnClouds20M);
    // The output file name
    totalWeightComputation20M.SetTotalWeightOutputFileName(outTotalWeight20M);
    totalWeightComputation20M.Update();
    totalWeightComputation20M.WriteToOutputFile();

}
