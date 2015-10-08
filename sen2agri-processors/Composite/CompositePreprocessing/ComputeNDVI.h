#ifndef COMPUTE_NDVI_H
#define COMPUTE_NDVI_H

#include "otbBandMathImageFilter.h"
#include "otbMultiToMonoChannelExtractROI.h"

#include "../../MACCSMetadata/include/MACCSMetadataReader.hpp"

#include "otbImage.h"
#include "otbWrapperTypes.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkUnaryFunctorImageFilter.h"
#include "itkCastImageFilter.h"
#include "otbVectorImageToImageListFilter.h"
#include "itkResampleImageFilter.h"
#include "itkIndent.h"
#include <vector>
#include "libgen.h"

class ComputeNDVI
{
public:
    typedef float                                                                   PixelType;
    typedef otb::Image<PixelType, 2>                                                OutputImageType;
    //typedef otb::Wrapper::FloatVectorImageType                                      OutputImageType;
    typedef otb::Wrapper::FloatVectorImageType                                      ImageType;
    typedef otb::ImageFileReader<ImageType>                                         ImageReaderType;

    typedef otb::ImageList<OutputImageType>            ImageListType;
    typedef otb::ImageFileWriter<OutputImageType>      WriterType;
    typedef otb::VectorImageToImageListFilter<ImageType, ImageListType>             VectorImageToImageListType;
    typedef otb::MultiToMonoChannelExtractROI<otb::Wrapper::FloatVectorImageType::InternalPixelType,
                                              otb::Wrapper::FloatImageType::PixelType>    ExtractROIFilterType;
    typedef otb::ObjectList<ExtractROIFilterType>                           ExtractROIFilterListType;
    typedef otb::BandMathImageFilter<OutputImageType>                       BMFilterType;
    typedef itk::MACCSMetadataReader                                        MACCSMetadataReaderType;

public:
    ComputeNDVI();
    void DoInit(std::string &xml);
    OutputImageType::Pointer DoExecute();
    const char * GetNameOfClass() { return "ComputeNDVI"; }

private:
    std::string getMACCSRasterFileName(const std::string& rootFolder,
                                                    const std::vector<MACCSFileInformation>& imageFiles,
                                                    const std::string& ending);
private:
    ExtractROIFilterType::Pointer       m_ExtractROIFilter;
    ExtractROIFilterListType::Pointer   m_ChannelExtractorList;
    BMFilterType::Pointer               m_Filter;
    VectorImageToImageListType::Pointer m_ImageList;
    ImageReaderType::Pointer            m_InImage;
    std::string                         m_DirName;
    std::string                         m_inXml;

};

#endif // COMPUTE_NDVI
