#include "CreateS2AnglesRaster.h"

#define ANGLES_GRID_SIZE    23

CreateS2AnglesRaster::CreateS2AnglesRaster()
{

}

void CreateS2AnglesRaster::DoInit( int res, std::string &xml)
{
    m_inXml = xml;
    m_nOutRes = res;
}

CreateS2AnglesRaster::OutputImageType::Pointer CreateS2AnglesRaster::DoExecute()
{
   MACCSMetadataReaderType::Pointer maccsMetadataReader = MACCSMetadataReaderType::New();
   m_Resampler = 0;
    std::vector<char> buf(m_inXml.begin(), m_inXml.end());
    m_DirName = std::string(dirname(buf.data()));
    m_DirName += '/';
    auto meta = maccsMetadataReader->ReadMetadata(m_inXml);
    // check if it is a sentinel 2 product, otherwise -> exception
    if (meta == nullptr)
        itkExceptionMacro("THe metadata file could not be read !");

    if (meta->Header.FixedHeader.Mission.find("SENTINEL") == std::string::npos)
        itkExceptionMacro("Mission is not a SENTINEL !");

    if(m_nOutRes != 10 && m_nOutRes != 20)
        itkExceptionMacro("Accepted resolutions for Sentinel mission are 10 or 20 only!");

    m_AnglesRaster = OutputImageType::New();
    OutputImageType::IndexType start;

    start[0] =   0;  // first index on X
    start[1] =   0;  // first index on Y

    OutputImageType::SizeType size;

      size[0]  = 23;  // size along X
      size[1]  = 23;  // size along Y

    OutputImageType::RegionType region;

    region.SetSize(size);
    region.SetIndex(start);

    m_AnglesRaster->SetRegions(region);
    m_AnglesRaster->SetNumberOfComponentsPerPixel(10);
    m_AnglesRaster->Allocate();

    const auto &viewingAngles = ComputeViewingAngles(meta->ProductInformation.ViewingAngles);


    std::vector<size_t> bandsToExtract = {
        1, 2, 3, 7
    };
    for (int band = 0; band < 4; band++) {
        if(viewingAngles[bandsToExtract[band]].Angles.Zenith.Values.size() != ANGLES_GRID_SIZE ||
            viewingAngles[bandsToExtract[band]].Angles.Azimuth.Values.size() != ANGLES_GRID_SIZE )
            itkExceptionMacro("The width and/or height of computed angles from the xml file is/are not 23");
    }

    for(unsigned int i = 0; i < ANGLES_GRID_SIZE; i++) {
        for(unsigned int j = 0; j < ANGLES_GRID_SIZE; j++) {
            itk::VariableLengthVector<float> vct(10);
            vct[0] = meta->ProductInformation.SolarAngles.Zenith.Values[i][j];
            vct[1] = meta->ProductInformation.SolarAngles.Azimuth.Values[i][j];
            for (int band = 0; band < 4; band++) {
                vct[band * 2 + 2] = viewingAngles[bandsToExtract[band]].Angles.Zenith.Values[i][j];
                vct[band * 2 + 3] = viewingAngles[bandsToExtract[band]].Angles.Azimuth.Values[i][j];
            }

            OutputImageType::IndexType idx;
            idx[0] = j;
            idx[1] = i;
            m_AnglesRaster->SetPixel(idx, vct);
        }
    }
    m_AnglesRaster->UpdateOutputInformation();
    std::string resSuffix("_FRE_R1");
    if(m_nOutRes == 20)
        resSuffix = "_FRE_R2";
    std::string fileMetadata = getMACCSRasterFileName(m_DirName, meta->ProductOrganization.ImageFiles, resSuffix, true);
    std::cout << fileMetadata << std::endl;
    meta = maccsMetadataReader->ReadMetadata(fileMetadata);
    // check if it is a sentinel 2 product, otherwise -> exception
    if (meta == nullptr)
        itkExceptionMacro("The resolution metadata file could not be read !");

    int width = atoi(meta->ImageInformation.Size.Columns.c_str());
    int height = atoi(meta->ImageInformation.Size.Lines.c_str());
    if(width == 0 || height == 0)
        itkExceptionMacro("The read width/height from the resolution metadata file is/are 0");
    createResampler(m_AnglesRaster, width, height);

    return m_Resampler->GetOutput();
}

void CreateS2AnglesRaster::createResampler(const OutputImageType::Pointer& image, const int wantedWidth, const int wantedHeight) {

     m_Resampler = ResampleFilterType::New();
     m_Resampler->SetInput(image);

     // Set the interpolator
     LinearInterpolationType::Pointer interpolator = LinearInterpolationType::New();
     m_Resampler->SetInterpolator(interpolator);

     IdentityTransformType::Pointer transform = IdentityTransformType::New();

     m_Resampler->SetOutputParametersFromImage( image );
     // Scale Transform
     auto sz = image->GetLargestPossibleRegion().GetSize();
     OutputVectorType scale;
     scale[0] = (float)sz[0] / wantedWidth;
     scale[1] = (float)sz[1] / wantedHeight;

     // Evaluate spacing
     OutputImageType::SpacingType spacing = image->GetSpacing();
     OutputImageType::SpacingType OutputSpacing;
     OutputSpacing[0] = spacing[0] * scale[0];
     OutputSpacing[1] = spacing[1] * scale[1];

     m_Resampler->SetOutputSpacing(OutputSpacing);

     otb::Wrapper::FloatVectorImageType::PointType origin = image->GetOrigin();
     otb::Wrapper::FloatVectorImageType::PointType outputOrigin;
     outputOrigin[0] = origin[0] + 0.5 * spacing[0] * (scale[0] - 1.0);
     outputOrigin[1] = origin[1] + 0.5 * spacing[1] * (scale[1] - 1.0);

     m_Resampler->SetOutputOrigin(outputOrigin);

     m_Resampler->SetTransform(transform);

     ResampleFilterType::SizeType recomputedSize;
     recomputedSize[0] = wantedWidth;
     recomputedSize[1] = wantedHeight;

     m_Resampler->SetOutputSize(recomputedSize);
}

// Return the path to a file for which the name end in the ending
std::string CreateS2AnglesRaster::getMACCSRasterFileName(const std::string& rootFolder,
                                   const std::vector<MACCSFileInformation>& imageFiles,
                                   const std::string& ending,
                                   const bool fileTypeMeta) {

    for (const MACCSFileInformation& fileInfo : imageFiles) {
        if (fileInfo.LogicalName.length() >= ending.length() &&
                0 == fileInfo.LogicalName.compare (fileInfo.LogicalName.length() - ending.length(), ending.length(), ending)) {
            return rootFolder + fileInfo.FileLocation.substr(0, fileInfo.FileLocation.find_last_of('.')) + (fileTypeMeta ?  ".HDR" : ".DBL.TIF");
        }

    }
    return "";
}

