#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include "DirectionalCorrection.h"
#include "ResampleAtS2Res.h"
#include "ComputeNDVI.h"
#include "CreateS2AnglesRaster.h"

namespace otb
{
namespace Wrapper
{
class CompositePreprocessing : public Application
{
public:
    typedef CompositePreprocessing Self;
    typedef Application Superclass;
    typedef itk::SmartPointer<Self> Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;
    itkNewMacro(Self)

    itkTypeMacro(CompositePreprocessing, otb::Application)

    typedef otb::Wrapper::FloatVectorImageType                    InputImageType;
    typedef otb::Wrapper::FloatVectorImageType                    ImageType1;
    typedef float                                                 PixelType;
    typedef otb::Image<PixelType, 2>                              ImageType2;
    //typedef DirectionalCorrectionFilter<InputImageType, InputImageType >    DirectionalCorrectionFilterType;

private:

    void DoInit()
    {
        SetName("CompositePreprocessing");
        SetDescription("Resample the corresponding bands from LANDSAT or SPOT to S2 resolution");

        SetDocName("CompositePreprocessing");
        SetDocLongDescription("long description");
        SetDocLimitations("None");
        SetDocAuthors("CIU");
        SetDocSeeAlso(" ");

        AddParameter(ParameterType_String, "xml", "Input general L2A XML");
        AddParameter(ParameterType_Int, "res", "The resolution to be processed");
        SetDefaultParameterInt("res", -1);
        MandatoryOff("res");

        // Directional Correction parameters
        AddParameter(ParameterType_InputFilename, "scatcoef", "File containing coefficients for scattering function");
        MandatoryOff("scatcoef");
        AddParameter(ParameterType_String, "msk", "Image with 3 bands with cloud, water and snow masks");

        AddParameter(ParameterType_Int, "allinone", "Specifies if all bands should be resampled at 10m and 20m");
        SetDefaultParameterInt("allinone", 0);
        MandatoryOff("allinone");

        AddParameter(ParameterType_OutputImage, "outres", "Out Image at the original resolution");
        MandatoryOff("outres");
        AddParameter(ParameterType_OutputImage, "outcmres", "Out cloud mask image at the original  resolution");
        MandatoryOff("outcmres");
        AddParameter(ParameterType_OutputImage, "outwmres", "Out water mask image at the original  resolution");
        MandatoryOff("outwmres");
        AddParameter(ParameterType_OutputImage, "outsmres", "Out snow mask image at the original  resolution");
        MandatoryOff("outsmres");
        AddParameter(ParameterType_OutputImage, "outaotres", "Out snow mask image at the original  resolution");
        MandatoryOff("outaotres");

//      TODO: This is already in outres
//        AddParameter(ParameterType_OutputImage, "out", "Out image containing reflectances with directional correction.");

        SetDocExampleParameterValue("xml", "/path/to/L2Aproduct_maccs.xml");
        SetDocExampleParameterValue("msk", "/path/to/msks.tif");
        SetDocExampleParameterValue("allinone", "1");

        SetDocExampleParameterValue("outres", "/path/to/output_image.tif");
        SetDocExampleParameterValue("outcmres", "/path/to/output_image_cloud.tif");
        SetDocExampleParameterValue("outwmres", "/path/to/output_image_water.tif");
        SetDocExampleParameterValue("outsmres", "/path/to/output_image_snow.tif");
        SetDocExampleParameterValue("outaotres", "/path/to/output_image_aot.tif");

    }

    void DoUpdateParameters()
    {
      // Nothing to do.
    }

    void DoExecute()
    {
        std::string inXml = GetParameterAsString("xml");
        bool allInOne = (GetParameterInt("allinone") != 0);
        int res = GetParameterInt("res");

        auto factory = MetadataHelperFactory::New();
        auto pHelper = factory->GetMetadataHelper(inXml, res);
        std::string missionName = pHelper->GetMissionName();
        std::string mskImg = GetParameterAsString("msk");
        if(missionName.find(SENTINEL_MISSION_STR) != std::string::npos) {
            std::string scatCoeffsFile = GetParameterAsString("scatcoef");
            m_computeNdvi.DoInit(inXml);
            m_creatAngles.DoInit(res, inXml);
            ImageType1::Pointer anglesImg = m_creatAngles.DoExecute();
            ImageType2::Pointer ndviImg = m_computeNdvi.DoExecute();
            m_dirCorr.Init(res, inXml, scatCoeffsFile, mskImg, anglesImg, ndviImg);
            m_dirCorr.DoExecute();
            SetParameterOutputImage("outres", m_dirCorr.GetResampledMainImg().GetPointer());
            SetParameterOutputImage("outcmres", m_dirCorr.GetResampledCloudMaskImg().GetPointer());
            SetParameterOutputImage("outwmres", m_dirCorr.GetResampledWaterMaskImg().GetPointer());
            SetParameterOutputImage("outsmres", m_dirCorr.GetResampledSnowMaskImg().GetPointer());
            SetParameterOutputImage("outaotres", m_dirCorr.GetResampledAotImg().GetPointer());
        } else if ((missionName.find(LANDSAT_MISSION_STR) != std::string::npos) ||
                   (missionName.find(SPOT4_MISSION_STR) != std::string::npos)) {
            m_resampleAtS2Res.Init(allInOne, inXml, mskImg, res);
            m_resampleAtS2Res.DoExecute();

            SetParameterOutputImage("outres", m_resampleAtS2Res.GetResampledMainImg());
            SetParameterOutputImage("outcmres", m_resampleAtS2Res.GetResampledCloudMaskImg().GetPointer());
            SetParameterOutputImage("outwmres", m_resampleAtS2Res.GetResampledWaterMaskImg().GetPointer());
            SetParameterOutputImage("outsmres", m_resampleAtS2Res.GetResampledSnowMaskImg().GetPointer());
            SetParameterOutputImage("outaotres", m_resampleAtS2Res.GetResampledAotImg().GetPointer());
        } else {
            itkExceptionMacro("Unknown sensor type " << missionName);
        }
    }

private:
    ComputeNDVI             m_computeNdvi;
    CreateS2AnglesRaster    m_creatAngles;
    DirectionalCorrection   m_dirCorr;
    ResampleAtS2Res         m_resampleAtS2Res;

};
}
}

OTB_APPLICATION_EXPORT(otb::Wrapper::CompositePreprocessing)



