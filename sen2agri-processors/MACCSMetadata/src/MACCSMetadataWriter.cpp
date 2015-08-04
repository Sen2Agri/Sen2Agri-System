#include "MACCSMetadataWriter.hpp"

namespace itk
{
XDocument MACCSMetadataWriter::CreateMetadataXml(const MACCSFileMetadata &metadata)
{
    XDocument doc(XDeclaration("1.0", "UTF-8", ""),
                  XUnknown("<?xml-stylesheet type=\"text/xsl\" href=\"DISPLAY/display.xsl\"?>"));

    XElement root("Earth_Explorer_Header",
                  XAttribute("xmlns", "http://eop-cfi.esa.int/CFI"),
                  XAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance"),
                  XAttribute("schema_version", metadata.Header.SchemaVersion),
                  XAttribute("xsi:schemaLocation", metadata.Header.SchemaLocation),
                  XAttribute("xsi:type", metadata.Header.Type),
                  XElement("Fixed_Header",
                           XElement("File_Name", metadata.Header.FileName),
                           XElement("File_Description", metadata.Header.FileDescription),
                           XElement("Notes", metadata.Header.Notes),
                           XElement("Mission", metadata.Header.Mission),
                           XElement("File_Class", metadata.Header.FileClass),
                           XElement("File_Type", metadata.Header.FileType),
                           XElement("Validity_Period",
                                    XElement("Validity_Start", metadata.Header.ValidityStart),
                                    XElement("Validity_Stop", metadata.Header.ValidityStop)),
                           XElement("File_Version", metadata.Header.FileVersion),
                           XElement("Source",
                                    XElement("System", metadata.Header.SourceSystem),
                                    XElement("Creator", metadata.Header.Creator),
                                    XElement("Creator_Version", metadata.Header.CreatorVersion),
                                    XElement("Creation_Date", metadata.Header.CreationDate))));

    XElement bands("List_of_Bands",
                   XAttribute("count", std::to_string(metadata.ImageInformation.Bands.size())));
    for (const auto &band : metadata.ImageInformation.Bands) {
        bands.Append(XElement("Band", XAttribute("sn", band.Id), XText(band.Name)));
    }

    root.Append(XElement(
        "Variable_Header",
        XElement("Main_Product_Header",
                 // NOTE: we don't know the schema, so we can't emit these
                 XElement("List_of_Consumers", XAttribute("count", "0")),
                 XElement("List_of_Extensions", XAttribute("count", "0"))),
        XElement(
            "Specific_Product_Header",
            XElement("Instance_Id",
                     XElement("Reference_Product_Semantic",
                              metadata.InstanceId.ReferenceProductSemantic),
                     XElement("Reference_Product_Instance",
                              metadata.InstanceId.ReferenceProductInstance),
                     XElement("Annex_Code", metadata.InstanceId.AnnexCode)),
            XElement("Reference_Product_Header_Id", metadata.ReferenceProductHeaderId),
            XElement("Annex_Complete_Name", metadata.AnnexCompleteName),
            XElement(
                metadata.ImageInformation.ElementName,
                XElement("Format", metadata.ImageInformation.Format),
                XElement("Binary_Encoding", metadata.ImageInformation.BinaryEncoding),
                XElement("Data_Type", metadata.ImageInformation.DataType),
                XElement("Number_of_Significant_Bits",
                         metadata.ImageInformation.NumberOfSignificantBits),
                XElement("Nodata_Value", metadata.ImageInformation.NoDataValue),
                XElement("VAP_Nodata_Value", metadata.ImageInformation.VAPNoDataValue),
                XElement("VAP_Quantification_Value",
                         metadata.ImageInformation.VAPQuantificationValue),
                XElement("AOT_Nodata_Value", metadata.ImageInformation.AOTNoDataValue),
                XElement("AOT_Quantification_Value",
                         metadata.ImageInformation.AOTQuantificationValue),
                XElement("Size",
                         XElement("Lines", metadata.ImageInformation.SizeLines),
                         XElement("Columns", metadata.ImageInformation.SizeColumns),
                         XElement("Bands", metadata.ImageInformation.SizeBands)),
                XElement("Image_Compacting_Tool", metadata.ImageInformation.ImageCompactingTool),
                std::move(bands),
                XElement("Subsampling_Factor",
                         XElement("By_Line", metadata.ImageInformation.SubSamplingFactorLine),
                         XElement("By_Column", metadata.ImageInformation.SubSamplingFactorColumn),
                         XText(metadata.ImageInformation.SubSamplingFactor)),
                XElement("Values_Unit", metadata.ImageInformation.ValuesUnit),
                XElement("Quantification_Bit_Value",
                         metadata.ImageInformation.QuantificationBitValue),
                XElement("Colorspace", metadata.ImageInformation.ColorSpace),
                XElement("Bands_Order", metadata.ImageInformation.BandsOrder)))));

    doc.Append(root);

    return doc;
}

void MACCSMetadataWriter::WriteMetadata(const MACCSFileMetadata &metadata, const std::string &path)
{
    CreateMetadataXml(metadata).Save(path);
}
}
