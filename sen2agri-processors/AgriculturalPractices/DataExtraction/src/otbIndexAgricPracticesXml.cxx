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

#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>

#include "tinyxml_utils.hpp"
#include "string_utils.hpp"
#include "CommonFunctions.h"
#include <inttypes.h>
#include <boost/iostreams/device/mapped_file.hpp> // for mmap
#include <algorithm>  // for std::find
#include <iostream>   // for std::cout
#include <cstring>

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <errno.h>

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>


namespace otb
{
namespace Wrapper
{
class IndexAgricPracticesXml : public Application
{
public:
    /** Standard class typedefs. */
    typedef IndexAgricPracticesXml        Self;
    typedef Application                   Superclass;
    typedef itk::SmartPointer<Self>       Pointer;
    typedef itk::SmartPointer<const Self> ConstPointer;

    /** Standard macro */
    itkNewMacro(Self);

    itkTypeMacro(IndexAgricPracticesXml, otb::Application);


    IndexAgricPracticesXml()
    {
        //const std::string &regex = R"(\s?<fid id=\"(.*)\" name=\"(.*)\">)";
        const std::string &regex = R"(\s?<fid id=\"(.*)\">)";
        boost::regex regexExp {regex};
        m_startFidRegexExp = regexExp;
    }

    void DoInit() override
    {
        SetName("IndexAgricPracticesXml");
        SetDescription("Indexes an Agricultural Practices data extraction XML.");

        // Documentation
        SetDocName("Polygon Class Statistics");
        SetDocLongDescription("TODO");
        SetDocLimitations("None");
        SetDocAuthors("OTB-Team");
        SetDocSeeAlso(" ");

        AddDocTag(Tags::Learning);


        AddParameter(ParameterType_String, "in", "Input file");
        SetParameterDescription("in", "The input agricultural practices data extraction file to be indexed");
        MandatoryOff("in");

        AddRAMParameter();

        // Doc example parameter settings
        SetDocExampleParameterValue("xml", "file1.xml");

        //SetOfficialDocLink();
    }

    void DoUpdateParameters() override
    {

    }


    void DoExecute() override
    {
        const std::string &inFile = this->GetParameterString("in");
        if(inFile.size() == 0) {
            otbAppLogFATAL(<<"No file was given as input!");
        }

        boost::filesystem::path path(inFile);
        std::string ext = path.extension().string();
        bool bIsCsv = false;
        if (boost::iequals(ext, ".csv")) {
            bIsCsv = true;
        } else if (!boost::iequals(ext, ".xml")) {
            otbAppLogFATAL(<<"Invalid extension of input files " << ext);
        }

        std::ofstream xmlIndexFile;
        std::string outIdxPath = (path.parent_path() / path.filename()).string() + ".idx";
        xmlIndexFile.open(outIdxPath, std::ios_base::trunc | std::ios_base::out);

        struct stat sb;
        uintmax_t cntr = 0;
        int fd, lineLen;
        char *line;
        // map the file
        fd = open(inFile.c_str(), O_RDONLY);
        fstat(fd, &sb);
        //// int pageSize;
        //// pageSize = getpagesize();
        //// data = mmap((caddr_t)0, pageSize, PROT_READ, MAP_PRIVATE, fd, pageSize);
        char *data = static_cast<char*>(mmap((caddr_t)0, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0));

        uintmax_t curLineStart;
        uintmax_t curFidStart = 0;
        // get lines
        std::string curFid;
        std::string curName;
        while(cntr < (uintmax_t)sb.st_size) {
            lineLen = 0;
            line = data;
            curLineStart = cntr;
            // find the next line
            while(*data != '\n' && cntr < (uintmax_t) sb.st_size) {
                data++;
                cntr++;
                lineLen++;
            }
            char buf[lineLen+1];
            strncpy(buf, line, lineLen);
            buf[lineLen] = 0;
            std::string strLine(buf);

            if (bIsCsv) {
                if (!strstr(buf, ";suffix;")) {
                    curFidStart = curLineStart;
                    const std::vector<std::string> &lineElems = split(strLine, ';');
                    // get the first two elements
                    curFid = lineElems[0] + ";" + lineElems[1];
                    curFidStart = curLineStart;
                    WriteIndexLine(xmlIndexFile, curFid, "", curFidStart, cntr - curFidStart + 1);
                }
            } else {
                //const std::string &regex = "<fid id=\"(.*)\" name=\"(.*)\">";
                const std::string &regex = "<fid id=\"(.*)\">";
                boost::regex regexExp {regex};
                boost::smatch matches;
                if (boost::regex_search(strLine,matches,regexExp)) {
                    curFidStart = curLineStart;
                    curFid = matches[1].str();
                    //curName = matches[2].str();
                } else if (strstr(buf, "</fid>")) {
                    // end of the tag
                    if (curFid.size()/* && curName.size()*/) {
                        WriteIndexLine(xmlIndexFile, curFid, curName, curFidStart, cntr - curFidStart + 1);
                    }
                }
            }
            // skip the found \n
            data++;
            cntr++;
        }
    }

    void WriteIndexLine(std::ofstream &outIdxStream, const std::string &fid, const std::string &name,
                        const uintmax_t &curFileIdx, size_t byteToWrite) {
        (void)name;
        if (outIdxStream.is_open()) {
            outIdxStream << fid.c_str() << ";" /*<< name.c_str() << ";" */ <<
                            curFileIdx << ";" << byteToWrite <<"\n";
        }
    }
private:
     boost::regex m_startFidRegexExp;
};

} // end of namespace Wrapper
} // end of namespace otb

OTB_APPLICATION_EXPORT(otb::Wrapper::IndexAgricPracticesXml)
