/*=========================================================================
  *
  * Program:      Sen2agri-Processors
  * Language:     C++
  * Copyright:    2015-2016, CS Romania, office@c-s.ro
  * See COPYRIGHT file for details.
  *
  * Unless required by applicable law or agreed to in writing, software
  * distributed under the License is distributed on an "AS IS" BASIS,
  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  * See the License for the specific language governing permissions and
  * limitations under the License.

 =========================================================================*/
 
#include "weightonaot.h"
#include "cloudmaskbinarization.h"
#include "cloudsinterpolation.h"
#include "gaussianfilter.h"
#include "cloudweightcomputation.h"
#include "totalweightcomputation.h"
#include "../Common/MetadataHelperFactory.h"


std::string l8GenXmlFile ("/media/sf_2_Sen2Agri/8_ProductSamples/2_FromMACCS/L8_TEST_L8C_L2VALD_198030_20130626.HDR");
std::string s2GenXmlFile ("/media/sf_2_Sen2Agri/8_ProductSamples/2_FromMACCS/S2A_OPER_SSC_L2VALD_15SVD____20091211.HDR");
std::string spotXmlFile ("/media/sf_2_Sen2Agri/8_ProductSamples/3_Spot4/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000/SPOT4_HRVIR1_XS_20130318_N2A_EBelgiumD0000B0000.xml");

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

void TestXmlLoading();
void TestWeightOnAOT();
void TestComputeCloudWeight();
void TestTotalWeight();

bool g_bDebug = false;
bool g_allResolutions = false;

int main(int argc, char* argv[])
{
    if(argc > 1) {
        if(strcmp(argv[1], "-d") == 0)
        {
            g_bDebug = true;
            if(argc > 2)
            {
                if(strcmp(argv[2], "all") == 0)
                {
                    g_allResolutions = true;
                }
            }
        }
    }
    TestXmlLoading();
    TestWeightOnAOT();
    TestComputeCloudWeight();
    TestTotalWeight();
}

void TestXmlLoading()
{
    //auto pHelper;
    std::cout << "Loading AOT from XML " << l8GenXmlFile << std::endl;
    auto factory = new MetadataHelperFactory();
    auto pHelper = factory->GetMetadataHelper(l8GenXmlFile, 10);
    std::cout << "======================" << std::endl;
    std::cout << "AOT Image File 10m " << pHelper->GetAotImageFileName() << std::endl;
    std::cout << "AOT Band No " << pHelper->GetAotBandIndex() << std::endl;
    std::cout << "AOT Quantification Value " << pHelper->GetAotQuantificationValue() << std::endl;
    std::cout << "AOT No data value " << pHelper->GetAotNoDataValue() << std::endl;

    pHelper = factory->GetMetadataHelper(l8GenXmlFile, 20);
    std::cout << "======================" << std::endl;
    std::cout << "AOT Image File 20m " << pHelper->GetAotImageFileName() << std::endl;
    std::cout << "AOT Band No " << pHelper->GetAotBandIndex() << std::endl;
    std::cout << "AOT Quantification Value " << pHelper->GetAotQuantificationValue() << std::endl;
    std::cout << "AOT No data value " << pHelper->GetAotNoDataValue() << std::endl;

    std::cout << std::endl << "Loading AOT from XML " << s2GenXmlFile << std::endl;
    pHelper = factory->GetMetadataHelper(s2GenXmlFile, 10);
    std::cout << "======================" << std::endl;
    std::cout << "AOT Image File 10m " << pHelper->GetAotImageFileName() << std::endl;
    std::cout << "AOT Band No " << pHelper->GetAotBandIndex() << std::endl;
    std::cout << "AOT Quantification Value " << pHelper->GetAotQuantificationValue() << std::endl;
    std::cout << "AOT No data value " << pHelper->GetAotNoDataValue() << std::endl;

    pHelper = factory->GetMetadataHelper(s2GenXmlFile, 20);
    std::cout << "======================" << std::endl;
    std::cout << "AOT Image File 20m " << pHelper->GetAotImageFileName() << std::endl;
    std::cout << "AOT Band No " << pHelper->GetAotBandIndex() << std::endl;
    std::cout << "AOT Quantification Value " << pHelper->GetAotQuantificationValue() << std::endl;
    std::cout << "AOT No data value " << pHelper->GetAotNoDataValue() << std::endl;

    std::cout << std::endl << "Loading AOT from XML " << spotXmlFile << std::endl;
    pHelper = factory->GetMetadataHelper(spotXmlFile, 10);
    std::cout << "======================" << std::endl;
    std::cout << "AOT Image File 10m " << pHelper->GetAotImageFileName() << std::endl;
    std::cout << "AOT Band No " << pHelper->GetAotBandIndex() << std::endl;
    std::cout << "AOT Quantification Value " << pHelper->GetAotQuantificationValue() << std::endl;
    std::cout << "AOT No data value " << pHelper->GetAotNoDataValue() << std::endl;

    pHelper = factory->GetMetadataHelper(spotXmlFile, 20);
    std::cout << "======================" << std::endl;
    std::cout << "AOT Image File 20m " << pHelper->GetAotImageFileName() << std::endl;
    std::cout << "AOT Band No " << pHelper->GetAotBandIndex() << std::endl;
    std::cout << "AOT Quantification Value " << pHelper->GetAotQuantificationValue() << std::endl;
    std::cout << "AOT No data value " << pHelper->GetAotNoDataValue() << std::endl;
}

void TestWeightOnAOT()
{
    std::cout << "Computing WeightAOT 10m" << std::endl;
    WeightOnAOT weightOnAot10M;
    weightOnAot10M.SetInputFileName(inAotFile10M);
    weightOnAot10M.SetOutputFileName(outAOTWeight10M);
    weightOnAot10M.SetBand(2);
    weightOnAot10M.SetAotQuantificationValue(0.005);
    weightOnAot10M.SetAotMaxValue(50);
    weightOnAot10M.SetMinAotWeight(0.33);
    weightOnAot10M.SetMaxAotWeight(1);
    weightOnAot10M.WriteToOutputFile();

    if(g_allResolutions)
    {
        std::cout << "Computing WeightAOT 20m" << std::endl;
        WeightOnAOT weightOnAot20M;
        weightOnAot20M.SetInputFileName(inAotFile20M);
        weightOnAot20M.SetOutputFileName(outAOTWeight20M);
        weightOnAot20M.SetBand(2);
        weightOnAot20M.SetAotQuantificationValue(0.005);
        weightOnAot20M.SetAotMaxValue(50);
        weightOnAot20M.SetMinAotWeight(0.33);
        weightOnAot20M.SetMaxAotWeight(1);
        weightOnAot20M.WriteToOutputFile();
    }
}

void TestComputeCloudWeight()
{
    int inputCloudMaskResolution = 10;
    int coarseResolution = 240;
    float sigmaSmallCloud = 10.0;
    float sigmaLargeCloud = 100.0;
    long inImageWidth, inImageHeight;

    std::cout << "Performing CloudMaskBinarization " << std::endl;
    CloudMaskBinarization cloudMaskBinarization;
    cloudMaskBinarization.SetInputFileName(inFile);
    cloudMaskBinarization.SetOutputFileName(outFileBinarize);
    std::cout << "Input Cloud image Resolution is: " << cloudMaskBinarization.GetInputImageResolution() << std::endl;
    if(g_bDebug) {
        cloudMaskBinarization.WriteToOutputFile();
    }

    // perform undersampling to lower resolution
    std::cout << "Performing Undersampling " << std::endl;
    CloudsInterpolation underSampler;
    //underSampler.SetInputFileName(inFile);
    //underSampler.SetInputImage(cloudMaskBinarization.GetProducedImage());
    underSampler.SetInputImageReader(cloudMaskBinarization.GetOutputImageSource());
    underSampler.SetInputResolution(inputCloudMaskResolution);
    underSampler.SetOutputResolution(coarseResolution);
    underSampler.SetOutputFileName(outFileUndersample240M);
    underSampler.GetInputImageDimension(inImageWidth, inImageHeight);
    // By default, the undersampler is using the Bicubic interpolator with a radius of 2
    //undersampler.SetInterpolator(Interpolator_BCO);
    //undersampler.SetBicubicInterpolatorRadius(2);
    if(g_bDebug) {
        underSampler.WriteToOutputFile();
    }

    // Compute the DistLargeCloud, Low Res
    GaussianFilter gaussianFilterSmallCloud;
    std::cout << "Performing GaussianFilterSmallCloud " << std::endl;
    //gaussianFilterSmallCloud.SetInputImage(underSampler.GetProducedImage());
    gaussianFilterSmallCloud.SetInputImageReader(underSampler.GetOutputImageSource());
    //gaussianFilter1.SetInputFileName(inTest1);
    gaussianFilterSmallCloud.SetSigma(sigmaSmallCloud);
    gaussianFilterSmallCloud.SetOutputFileName(outFileGaussianSmallCld);
    if(g_bDebug) {
        gaussianFilterSmallCloud.WriteToOutputFile();
    }

    GaussianFilter gaussianFilterLargeCloud;
    std::cout << "Performing GaussianFilterLargeCloud " << std::endl;
    //gaussianFilter2.SetInputFileName(inTest2);
    //gaussianFilterLargeCloud.SetInputImage(underSampler.GetProducedImage());
    gaussianFilterLargeCloud.SetInputImageReader(underSampler.GetOutputImageSource());
    gaussianFilterLargeCloud.SetSigma(sigmaLargeCloud);
    gaussianFilterLargeCloud.SetOutputFileName(outFileGaussianLargeCld);
    if(g_bDebug) {
        gaussianFilterLargeCloud.WriteToOutputFile();
    }

    // Resample at full resolution of 10m and 20m
    for(int res = 10; res < 22; res++) {
        // resample at the current small resolution (10 or 20) the small cloud large resolution image
        std::cout << "Performing Resampling at " << ((res == 10) ? "10m" : "20m") << std::endl;
        CloudsInterpolation overSamplerSmallCloud;
        //overSamplerSmallCloud.SetInputImage(gaussianFilterSmallCloud.GetProducedImage());
        //overSampler1.SetInputFileName(outFileInt2);
        overSamplerSmallCloud.SetInputImageReader(gaussianFilterSmallCloud.GetOutputImageSource());
        overSamplerSmallCloud.SetOutputFileName(res == 10 ? outFileOversampleSmallCld10M : outFileOversampleSmallCld20M);
        overSamplerSmallCloud.SetInputResolution(coarseResolution);
        overSamplerSmallCloud.SetOutputResolution(res);
        overSamplerSmallCloud.SetInterpolator(Interpolator_Linear);
        overSamplerSmallCloud.SetOutputForcedSize(inImageWidth, inImageHeight);
        if(g_bDebug) {
            overSamplerSmallCloud.WriteToOutputFile();
        }

        std::cout << "Performing Resampling at " << ((res == 10) ? "10m" : "20m") << std::endl;
        // resample at the current small resolution (10 or 20) the large cloud large resolution image
        CloudsInterpolation overSamplerLargeCloud;
        //overSamplerLargeCloud.SetInputImage(gaussianFilterLargeCloud.GetProducedImage());
        //overSampler2.SetInputFileName(outFileInt3);
        overSamplerLargeCloud.SetInputImageReader(gaussianFilterLargeCloud.GetOutputImageSource());
        overSamplerLargeCloud.SetOutputFileName(res == 10 ? outFileOversampleLargeCld10M : outFileOversampleLargeCld20M);
        overSamplerLargeCloud.SetInputResolution(coarseResolution);
        overSamplerLargeCloud.SetOutputResolution(res);
        overSamplerLargeCloud.SetInterpolator(Interpolator_Linear);
        overSamplerLargeCloud.SetOutputForcedSize(inImageWidth, inImageHeight);
        if(g_bDebug) {
            overSamplerLargeCloud.WriteToOutputFile();
        }

        // compute the weight on clouds
        std::cout << "Performing CloudWeightComputation at " << ((res == 10) ? "10m" : "20m") << std::endl;
        CloudWeightComputation cloudWeightComputation;
        //weightOnClouds.SetInput1(overSamplerSmallCloud.GetProducedImage());
        //weightOnClouds.SetInput2(overSamplerLargeCloud.GetProducedImage());
        cloudWeightComputation.SetInputImageReader1(overSamplerSmallCloud.GetOutputImageSource());
        cloudWeightComputation.SetInputImageReader2(overSamplerLargeCloud.GetOutputImageSource());
        cloudWeightComputation.SetOutputFileName(res == 10 ? outWeightOnClouds10M : outWeightOnClouds20M);
        cloudWeightComputation.WriteToOutputFile();

        res += 10;
        // if not all resolutions, exit the loop after the first one
        if(!g_allResolutions)
        {
            break;
        }
    }

}

void TestTotalWeight()
{
    std::string missionName("SENTINEL-2A");
    float weightSensor = 0.33;
    std::string l2aDate = "20140103";
    std::string l3aDate = "20141110";
    int halfSynthesis = 50;
    float weightOnDateMin = 0.10;

    std::cout << "Performing TotalWeightComputation at 10m" << std::endl;
    TotalWeightComputation totalWeightComputation10M;
    // weight on sensor parameters
    totalWeightComputation10M.SetMissionName(missionName);
    totalWeightComputation10M.SetWeightOnSensor(weightSensor);
    // weight on date parameters
    totalWeightComputation10M.SetDates(l2aDate, l3aDate);
    totalWeightComputation10M.SetHalfSynthesisPeriodAsDays(halfSynthesis);
    totalWeightComputation10M.SetWeightOnDateMin(weightOnDateMin);
    // Weights for AOT and Clouds
    totalWeightComputation10M.SetAotWeightFile(outAOTWeight10M);
    totalWeightComputation10M.SetCloudsWeightFile(outWeightOnClouds10M);
    // The output file name
    totalWeightComputation10M.SetTotalWeightOutputFileName(outTotalWeight10M);
    totalWeightComputation10M.WriteToOutputFile();

    if(g_allResolutions)
    {
        std::cout << "Performing TotalWeightComputation at 20m" << std::endl;
        TotalWeightComputation totalWeightComputation20M;
        // weight on sensor parameters
        totalWeightComputation20M.SetMissionName(missionName);
        totalWeightComputation20M.SetWeightOnSensor(weightSensor);
        // weight on date parameters
        totalWeightComputation20M.SetDates(l2aDate, l3aDate);
        totalWeightComputation20M.SetHalfSynthesisPeriodAsDays(halfSynthesis);
        totalWeightComputation20M.SetWeightOnDateMin(weightOnDateMin);
        // Weights for AOT and Clouds
        totalWeightComputation20M.SetAotWeightFile(outAOTWeight20M);
        totalWeightComputation20M.SetCloudsWeightFile(outWeightOnClouds20M);
        // The output file name
        totalWeightComputation20M.SetTotalWeightOutputFileName(outTotalWeight20M);
        totalWeightComputation20M.WriteToOutputFile();
    }
}
