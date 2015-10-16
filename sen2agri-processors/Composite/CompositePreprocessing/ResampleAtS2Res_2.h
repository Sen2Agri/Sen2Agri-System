#ifndef RESAMPLE_AT_S2_RES_H
#define RESAMPLE_AT_S2_RES_H

#include "otbWrapperTypes.h"
#include "otbMultiToMonoChannelExtractROI.h"
#include "otbStreamingResampleImageFilter.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "otbImageListToVectorImageFilter.h"
#include "libgen.h"
//Transform
#include "itkComposeImageFilter.h"
#include "itkScalableAffineTransform.h"
#include "../Common/ResampledBandExtractor.h"
#include "MetadataHelperFactory.h"

#include "BandsCfgMappingParser.h"

class ResampleAtS2Res2
{
public:
    typedef float                                      PixelType;
    typedef otb::VectorImage<PixelType, 2>             ImageType;
    typedef itk::VectorImage<PixelType, 2>             ImageType2;
    typedef otb::Image<PixelType, 2>                   InternalImageType;
    typedef otb::ImageList<ImageType>                  ImageListType;
    typedef otb::ImageList<InternalImageType>          InternalImageListType;

    typedef otb::ImageFileReader<ImageType>                            ImageReaderType;
    typedef otb::ObjectList<ImageReaderType>                           ImageReaderListType;
    typedef otb::ImageListToVectorImageFilter<InternalImageListType,
                                         ImageType >                   ListConcatenerFilterType;

public:
    ResampleAtS2Res2();
    void Init(const std::string &xml, const std::string &strMaskFileName,
              const std::string &bandsMappingFile, int nRes);
    void DoExecute();

    ImageType::Pointer GetResampledMainImg();
    InternalImageType::Pointer GetResampledCloudMaskImg();
    InternalImageType::Pointer GetResampledWaterMaskImg();
    InternalImageType::Pointer GetResampledSnowMaskImg();
    InternalImageType::Pointer GetResampledAotImg();

    const char * GetNameOfClass() { return "ResampleAtS2Res2"; }

private:
    ImageReaderType::Pointer getReader(const std::string& filePath);
    bool ExtractResampledMasksImages();
    void ExtractResampledAotImage();

private:
    std::string m_strXml;
    std::string m_strMaskFileName;

    ListConcatenerFilterType::Pointer     m_Concatener;

    ImageReaderListType::Pointer          m_ImageReaderList;
    InternalImageListType::Pointer        m_ImageList;

    InternalImageType::Pointer            m_ImageCloud;
    InternalImageType::Pointer            m_ImageWater;
    InternalImageType::Pointer            m_ImageSnow;
    InternalImageType::Pointer            m_ImageAot;

    int m_nRes;
    ImageReaderType::Pointer m_inputImgReader;

    ResampledBandExtractor m_ResampledBandsExtractor;
    std::unique_ptr<MetadataHelper> m_pMetadataHelper;

    BandsCfgMappingParser m_bandsCfgMappingParser;
    std::string m_bandsMappingFile;

};

#endif // RESAMPLE_AT_S2_RES_H
