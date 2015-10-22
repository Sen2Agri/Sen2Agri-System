#include <boost/algorithm/string/predicate.hpp>
#include <boost/filesystem.hpp>

#include "MetadataUtil.hpp"
#include "MACCSMetadataReader.hpp"
#include "SPOT4MetadataReader.hpp"

std::string getRasterFile(const MACCSFileMetadata &metadata, const char *suffix)
{
    std::string file;

    for (const auto &fileInfo : metadata.ProductOrganization.ImageFiles) {
        if (boost::algorithm::ends_with(fileInfo.LogicalName, suffix)) {
            boost::filesystem::path p(metadata.ProductPath);
            p.remove_filename();
            p /= fileInfo.FileLocation;
            p.replace_extension(".DBL.TIF");
            file = p.string();
            break;
        }

    }

    return file;
}

std::string getRasterFile(const SPOT4Metadata &metadata, const std::string &file)
{
    boost::filesystem::path p(metadata.ProductPath);
    p.remove_filename();
    p /= file;
    return p.string();
}

std::string getMainRasterFile(const MACCSFileMetadata &metadata)
{
    auto raster = getRasterFile(metadata, "_FRE_R1");
    if (raster.empty()) {
        raster = getRasterFile(metadata, "_FRE");
    }
    if (raster.empty()) {
        raster = getRasterFile(metadata, "_SRE_R1");
    }
    if (raster.empty()) {
        raster = getRasterFile(metadata, "_SRE");
    }
    return raster;
}

std::string getMainRasterFile(const SPOT4Metadata &metadata)
{
    auto raster = getRasterFile(metadata, metadata.Files.OrthoSurfCorrPente);
    if (raster.empty()) {
        raster = getRasterFile(metadata, metadata.Files.OrthoSurfCorrEnv);
    }
    return raster;
}

std::string getMainRasterFile(const std::string &path)
{
    if (auto metadata = itk::MACCSMetadataReader::New()->ReadMetadata(path)) {
        return getMainRasterFile(*metadata);
    } else if (auto metadata = itk::SPOT4MetadataReader::New()->ReadMetadata(path)) {
        return getMainRasterFile(*metadata);
    } else {
        return path;
    }
}
