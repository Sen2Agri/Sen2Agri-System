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
 
#include "tinyxml_utils.hpp"

std::string GetAttribute(const TiXmlElement *element, const char *attributeName)
{
    if (const char *at = element->Attribute(attributeName)) {
        return at;
    }

    return std::string();
}

std::string GetText(const TiXmlElement *element)
{
    if (const char *text = element->GetText()) {
        return text;
    }

    return std::string();
}

std::string GetChildText(const TiXmlElement *element, const char *childName)
{
    if (auto el = element->FirstChildElement(childName)) {
        if (const char *text = el->GetText())
            return text;
    }

    return std::string();
}

std::string
 GetChildAttribute(const TiXmlElement *element, const char *childName, const char *attributeName)
{
    if (auto el = element->FirstChildElement(childName)) {
        if (const char *at = el->Attribute(attributeName)) {
            return at;
        }
    }

    return std::string();
}
