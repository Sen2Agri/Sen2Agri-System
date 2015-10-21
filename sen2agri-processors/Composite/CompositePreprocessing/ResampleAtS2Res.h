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
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"
#include "libgen.h"
//Transform
#include "itkComposeImageFilter.h"
#include "itkScalableAffineTransform.h"
#include "ResampledBandExtractor.h"
#include "MetadataHelperFactory.h"

class ResampleAtS2Res
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
    ResampleAtS2Res();
    void Init(bool bAllInOne, std::string &xml, std::string &strMaskFileName, int nRes);
    void DoExecute();

    ImageType::Pointer GetResampledMainImg();
    InternalImageType::Pointer GetResampledCloudMaskImg();
    InternalImageType::Pointer GetResampledWaterMaskImg();
    InternalImageType::Pointer GetResampledSnowMaskImg();
    InternalImageType::Pointer GetResampledAotImg();

    const char * GetNameOfClass() { return "ResampleAtS2Res"; }

private:
    std::string getSpot4AotFileName(const std::unique_ptr<SPOT4Metadata>& meta);
    ImageReaderType::Pointer getReader(const std::string& filePath);
    bool ProcessLANDSAT8(bool allInOne);
    bool ProcessSPOT4(bool allInOne);

    bool ExtractResampledMasksImages(int curRes);

private:
    bool m_bAllInOne;
    std::string m_strXml;
    std::string m_strMaskFileName;

    ListConcatenerFilterType::Pointer     m_ConcatenerRes10;
    ListConcatenerFilterType::Pointer     m_ConcatenerRes20;
    ListConcatenerFilterType::Pointer     m_ConcatenerResOrig;

    ImageReaderListType::Pointer          m_ImageReaderList;
    InternalImageListType::Pointer        m_ImageListRes10;
    InternalImageListType::Pointer        m_ImageListRes20;
    InternalImageListType::Pointer        m_ImageListResOrig;

    InternalImageType::Pointer            m_ImageCloudRes10;
    InternalImageType::Pointer            m_ImageWaterRes10;
    InternalImageType::Pointer            m_ImageSnowRes10;
    InternalImageType::Pointer            m_ImageAotRes10;

    InternalImageType::Pointer            m_ImageCloudRes20;
    InternalImageType::Pointer            m_ImageWaterRes20;
    InternalImageType::Pointer            m_ImageSnowRes20;
    InternalImageType::Pointer            m_ImageAotRes20;

    InternalImageType::Pointer            m_ImageCloudResOrig;
    InternalImageType::Pointer            m_ImageWaterResOrig;
    InternalImageType::Pointer            m_ImageSnowResOrig;
    InternalImageType::Pointer            m_ImageAotResOrig;

    std::string                           m_DirName;
    int m_nRes;

    ResampledBandExtractor m_ResampledBandsExtractor;
    std::unique_ptr<MetadataHelper> m_pMetadataHelper;
};

#endif // RESAMPLE_AT_S2_RES_H
