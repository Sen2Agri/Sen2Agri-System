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
 
#include "MACCSMetadataWriter.hpp"

namespace itk
{
XElement Format(const MACCSFixedHeader &fixedHeader)
{
    return XElement("Fixed_Header",
                    XElement("File_Name", fixedHeader.FileName),
                    XElement("File_Description", fixedHeader.FileDescription),
                    XElement("Notes", fixedHeader.Notes),
                    XElement("Mission", fixedHeader.Mission),
                    XElement("File_Class", fixedHeader.FileClass),
                    XElement("File_Type", fixedHeader.FileType),
                    XElement("Validity_Period",
                             XElement("Validity_Start", fixedHeader.ValidityStart),
                             XElement("Validity_Stop", fixedHeader.ValidityStop)),
                    XElement("File_Version", fixedHeader.FileVersion),
                    XElement("Source",
                             XElement("System", fixedHeader.SourceSystem),
                             XElement("Creator", fixedHeader.Creator),
                             XElement("Creator_Version", fixedHeader.CreatorVersion),
                             XElement("Creation_Date", fixedHeader.CreationDate)));
}

XElement Format(const MACCSMainProductHeader &)
{
    return XElement("Main_Product_Header",
                    // NOTE: we don't know the schema, so we can't emit these
                    XElement("List_of_Consumers", XAttribute("count", "0")),
                    XElement("List_of_Extensions", XAttribute("count", "0")));
}

XElement Format(const MACCSInstanceId &instanceId)
{
    return XElement("Instance_Id",
                    XElement("Reference_Product_Semantic", instanceId.ReferenceProductSemantic),
                    XElement("Reference_Product_Instance", instanceId.ReferenceProductInstance),
                    XElement("Annex_Code", instanceId.AnnexCode),
                    XElement("Nick_Name", instanceId.NickName),
                    XElement("Acquisition_Date", instanceId.AcquisitionDate));
}

XElement Format(const MACCSSize &size)
{
    return XElement("Size",
                    XElement("Lines", size.Lines),
                    XElement("Columns", size.Columns),
                    XElement("Bands", size.Bands));
}

XElement Format(const std::vector<MACCSBand> &bands)
{
    XElement bandsEl("List_of_Bands", XAttribute("count", std::to_string(bands.size())));

    for (const auto &band : bands) {
        bandsEl.Append(XElement("Band", XAttribute("sn", band.Id), XText(band.Name)));
    }

    return bandsEl;
}

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
                  Format(metadata.Header.FixedHeader));

    root.Append(XElement(
        "Variable_Header",
        Format(metadata.MainProductHeader),
        XElement(
            "Specific_Product_Header",
            Format(metadata.InstanceId),
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
                Format(metadata.ImageInformation.Size),
                XElement("Image_Compacting_Tool", metadata.ImageInformation.ImageCompactingTool),
                Format(metadata.ImageInformation.Bands),
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
