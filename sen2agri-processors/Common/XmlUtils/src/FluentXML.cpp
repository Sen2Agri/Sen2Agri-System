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
 
#include "FluentXML.hpp"

XNode::XNode(TiXmlNode *node) : node(node)
{
}

void XNode::LinkEndChild(TiXmlNode *child)
{
    node->LinkEndChild(child);
}

TiXmlDocument *XNode::AsDocument()
{
    return static_cast<TiXmlDocument *>(node.get());
}

const TiXmlDocument *XNode::AsDocument() const
{
    return static_cast<TiXmlDocument *>(node.get());
}

TiXmlElement *XNode::AsElement()
{
    return static_cast<TiXmlElement *>(node.get());
}

const TiXmlElement *XNode::AsElement() const
{
    return static_cast<TiXmlElement *>(node.get());
}

TiXmlNode *XNode::Node()
{
    return node.get();
}

const TiXmlNode *XNode::Node() const
{
    return node.get();
}

TiXmlNode *XNode::Release()
{
    return node.release();
}

void XDocument::Save(const char *path) const
{
    AsDocument()->SaveFile(path);
}

void XDocument::Save(const std::string &path) const
{
    AsDocument()->SaveFile(path);
}

XText::XText(std::string text) : text(std::move(text))
{
}

void XText::AppendTo(XNode &parent)
{
    if (!text.empty()) {
        parent.LinkEndChild(new TiXmlText(std::move(text)));
    }
}

void XElement::AppendTo(XNode &parent)
{
    if (AsElement()->FirstChild() || AsElement()->FirstAttribute()) {
        parent.LinkEndChild(Release());
    }
}

void XElement::SetAttribute(const char *name, const char *value)
{
    AsElement()->SetAttribute(name, value);
}

XAttribute::XAttribute(std::string name, std::string value)
    : name(std::move(name)), value(std::move(value))
{
}

void XAttribute::AppendTo(XElement &parent)
{
    parent.SetAttribute(name.c_str(), value.c_str());
}

XDeclaration::XDeclaration(const char *version, const char *encoding, const char *standalone)
    : declaration(new TiXmlDeclaration(version, encoding, standalone))
{
}

void XDeclaration::AppendTo(XNode &parent)
{
    parent.LinkEndChild(declaration.release());
}

XUnknown::XUnknown(const char *s) : unknown(new TiXmlUnknown)
{
    std::istringstream ss(s);
    ss >> *unknown;
}

void XUnknown::AppendTo(XNode &parent)
{
    parent.LinkEndChild(unknown.release());
}

std::ostream & operator<<(std::ostream &o, const XDocument &doc)
{
    return o << *doc.AsDocument();
}
