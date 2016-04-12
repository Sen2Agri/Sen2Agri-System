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
 
/*=========================================================================

  Program:   ORFEO Toolbox
  Language:  C++
  Date:      $Date$
  Version:   $Revision$


  Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
  See OTBCopyright.txt for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


//  Software Guide : BeginCommandLineArgs
//    INPUTS: {reference polygons}, {sample ratio}
//    OUTPUTS: {training polygons}, {validation_polygons}
//  Software Guide : EndCommandLineArgs


//  Software Guide : BeginLatex
// The sample selection consists in splitting the reference data into 2 disjoint sets, the training set and the validation set.
// These sets are composed of polygons, not individual pixels.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
#include "otbWrapperApplication.h"
#include "otbWrapperApplicationFactory.h"
#include "FluentXML.hpp"
//  Software Guide : EndCodeSnippet

namespace otb
{

//  Software Guide : BeginLatex
//  Application class is defined in Wrapper namespace.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
namespace Wrapper
{
//  Software Guide : EndCodeSnippet


//  Software Guide : BeginLatex
//
//  SampleSelection class is derived from Application class.
//
//  Software Guide : EndLatex

//  Software Guide : BeginCodeSnippet
class XMLStatistics : public Application
//  Software Guide : EndCodeSnippet
{
public:
  //  Software Guide : BeginLatex
  // The \code{ITK} public types for the class, the superclass and smart pointers.
  // Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  typedef XMLStatistics Self;
  typedef Application Superclass;
  typedef itk::SmartPointer<Self> Pointer;
  typedef itk::SmartPointer<const Self> ConstPointer;
  // Software Guide : EndCodeSnippet

  //  Software Guide : BeginLatex
  //  Invoke the macros necessary to respect ITK object factory mechanisms.
  //  Software Guide : EndLatex

  //  Software Guide : BeginCodeSnippet
  itkNewMacro(Self)
;

  itkTypeMacro(XMLStatistics, otb::Application)
;
  //  Software Guide : EndCodeSnippet


private:

  //  Software Guide : BeginLatex
  //  \code{DoInit()} method contains class information and description, parameter set up, and example values.
  //  Software Guide : EndLatex

  void DoInit()
  {

    // Software Guide : BeginLatex
    // Application name and description are set using following methods :
    // \begin{description}
    // \item[\code{SetName()}] Name of the application.
    // \item[\code{SetDescription()}] Set the short description of the class.
    // \item[\code{SetDocName()}] Set long name of the application (that can be displayed \dots).
    // \item[\code{SetDocLongDescription()}] This methods is used to describe the class.
    // \item[\code{SetDocLimitations()}] Set known limitations (threading, invalid pixel type \dots) or bugs.
    // \item[\code{SetDocAuthors()}] Set the application Authors. Author List. Format : "John Doe, Winnie the Pooh" \dots
    // \item[\code{SetDocSeeAlso()}] If the application is related to one another, it can be mentioned.
    // \end{description}
    // Software Guide : EndLatex

    //  Software Guide : BeginCodeSnippet
      SetName("XMLStatistics");
      SetDescription("Convert the confusion matrix and quality metrics to XML.");

      SetDocName("XMLStatistics");
      SetDocLongDescription("Convert the confusion matrix and quality metrics to XML");
      SetDocLimitations("None");
      SetDocAuthors("LBU");
      SetDocSeeAlso(" ");
    //  Software Guide : EndCodeSnippet


    //  Software Guide : BeginCodeSnippet
    AddParameter(ParameterType_InputFilename, "confmat", "The confusion matrix file");
    AddParameter(ParameterType_InputFilename, "quality", "The quality metrics file");
    AddParameter(ParameterType_String, "root", "The name of the root element for the document");

    AddParameter(ParameterType_OutputFilename, "out", "THe output XML file");



    //  Software Guide : BeginCodeSnippet
    SetDocExampleParameterValue("confmat", "confusion_matrix_validation.csv");
    SetDocExampleParameterValue("quality", "quality_metrics.txt");
    SetDocExampleParameterValue("out", "quality_data.xml");
    //  Software Guide : EndCodeSnippet
  }

  // Software Guide : BeginLatex
  // \code{DoUpdateParameters()} is called as soon as a parameter value change. Section \ref{sec:appDoUpdateParameters}
  // gives a complete description of this method.
  // Software Guide : EndLatex
  //  Software Guide :BeginCodeSnippet
  void DoUpdateParameters()
  {
      // Nothing to do.
  }
  //  Software Guide : EndCodeSnippet

  // Software Guide : BeginLatex
  // The algorithm consists in a random sampling without replacement of the polygons of each class with
  // probability p = sample_ratio value for the training set and
  // 1 - p for the validation set.
  // Software Guide : EndLatex
  //  Software Guide :BeginCodeSnippet




  void DoExecute()
  {
      // Build the xml document
      XDocument doc(XDeclaration("1.0", "UTF-8", ""));

      // Create the root
      XElement root(GetParameterString("root"),
                        XAttribute("xmlns:xsi", "http://www.w3.org/2001/XMLSchema-instance"));

      // Add the confusion matrix data
      AddConfusionMatrix(GetParameterString("confmat"), root);

      // Add the quality metrics data
      AddQualityMetrics(GetParameterString("quality"), root);

      // Append the root
      doc.Append(root);

      // write the file
      doc.Save(GetParameterAsString("out"));
  }
  //  Software Guide :EndCodeSnippet

  void AddConfusionMatrix(std::string cmFileName, XElement& root)
  {
      std::ifstream cmFile;
      cmFile.open(cmFileName);
      if (!cmFile.is_open()) {
          itkExceptionMacro("Can't open the confusion matrix file for reading!");
      }

      // Build the element
      XElement cm("ConfusionMatrix");

      std::string textline;

      // read the first line (#Reference labels (rows))
      std::getline(cmFile, textline);
      // search for ':'
      size_t beginIndex = textline.find(':');
      if (beginIndex == std::string::npos) {
          itkExceptionMacro("The confusion matrix file is invalid.");
      }

      // Create an element
      XElement refl("ReferenceLabels");
      // Extract the class labels
      beginIndex++;
      size_t endIndex;
      do {
          endIndex = textline.find(",", beginIndex);

          std::string label;
          if (endIndex != std::string::npos) {
              label = textline.substr(beginIndex, endIndex-beginIndex);
              beginIndex = endIndex+1;
          } else {
              label = textline.substr(beginIndex);
          }

          //Create one element for each label
          XElement lbl("RowLabel", label);
          refl.Append(lbl);

      } while (endIndex != std::string::npos);
      cm.Append(refl);

      // read the second line (#Produced labels (columns))
      std::getline(cmFile, textline);
      // search for ':'
      beginIndex = textline.find(':');
      if (beginIndex == std::string::npos) {
          itkExceptionMacro("The confusion matrix file is invalid.");
      }

      // Create an element
      XElement prodl("ProducedLabels");
      // Extract the class labels
      beginIndex++;
      do {
          endIndex = textline.find(",", beginIndex);

          std::string label;
          if (endIndex != std::string::npos) {
              label = textline.substr(beginIndex, endIndex-beginIndex);
              beginIndex = endIndex+1;
          } else {
              label = textline.substr(beginIndex);
          }

          //Create one element for each label
          XElement lbl("ColumnLabel", label);
          prodl.Append(lbl);

      } while (endIndex != std::string::npos);
      cm.Append(prodl);

      // Create the element that holds the rest of the data
      XElement data("Data");
      // read the rest of the lines
      while (std::getline(cmFile, textline) && textline.size() > 0) {
          // For each row create an element
          XElement row("Row");
          beginIndex = 0;
          do {
              endIndex = textline.find(",", beginIndex);

              std::string label;
              if (endIndex != std::string::npos) {
                  label = textline.substr(beginIndex, endIndex-beginIndex);
                  beginIndex = endIndex+1;
              } else {
                  label = textline.substr(beginIndex);
              }

              //Create one element for each label
              XElement lbl("Column", label);
              row.Append(lbl);

          } while (endIndex != std::string::npos);

          // Add the row to the data
          data.Append(row);
      }
      // Append the data to the confusion matrix
      cm.Append(data);

      // Add the element to the root
      root.Append(cm);
      // Close the confusion matrix file
      cmFile.close();
  }

  void AddQualityMetrics(std::string qltFileName, XElement& root)
  {
      size_t beginIndex;
      std::ifstream qltFile;
      qltFile.open(qltFileName);
      if (!qltFile.is_open()) {
          itkExceptionMacro("Can't open the quality metrics file for reading!");
      }

      // Build the element
      XElement qlt("QualityMetrics");

      std::string textline;
      // read the file
      while (std::getline(qltFile, textline)) {

          // Precision of a class
          if ((beginIndex = textline.find("Precision of class")) != std::string::npos) {
              BuildElement(beginIndex, "Precision", qlt, textline);
              continue;
          }

          // Recall of class
          if ((beginIndex = textline.find("Recall of class")) != std::string::npos) {
              BuildElement(beginIndex, "Recall", qlt, textline);
              continue;
          }

          // F-score of class
          if ((beginIndex = textline.find("F-score of class")) != std::string::npos) {
              BuildElement(beginIndex, "F-score", qlt, textline);
              continue;
          }

          // Kappa index
          if ((beginIndex = textline.find("Kappa index")) != std::string::npos) {
              size_t valueStart = textline.find(':', beginIndex);
              if (valueStart == std::string::npos) {
                  itkExceptionMacro("The quality metrics file is invalid!");
              }
              std::string value = textline.substr(valueStart+1);

              // Create the XML element
              XElement elem("Kappa", value);

              // Add it to the quality metrics
              qlt.Append(elem);
              continue;
          }

          // Overall accuracy index
          if ((beginIndex = textline.find("Overall accuracy index")) != std::string::npos) {
              size_t valueStart = textline.find(':', beginIndex);
              if (valueStart == std::string::npos) {
                  itkExceptionMacro("The quality metrics file is invalid!");
              }
              std::string value = textline.substr(valueStart+1);

              // Create the XML element
              XElement elem("Accuracy", value);

              // Add it to the quality metrics
              qlt.Append(elem);
              continue;
          }
      }

      // Add the element to the root
      root.Append(qlt);
      // Close the quality metrics file
      qltFile.close();
  }


  void BuildElement(size_t beginIndex, std::string name, XElement& qlt, std::string& textline)
  {
      size_t classStart = textline.find('[', beginIndex);
      if (classStart == std::string::npos) {
          itkExceptionMacro("The quality metrics file is invalid!");
      }
      size_t classEnd = textline.find(']', classStart);
      if (classEnd == std::string::npos) {
          itkExceptionMacro("The quality metrics file is invalid!");
      }
      std::string label = textline.substr(classStart+1, classEnd-classStart-1);

      size_t valueStart = textline.find(':', classEnd);
      if (valueStart == std::string::npos) {
          itkExceptionMacro("The quality metrics file is invalid!");
      }
      std::string value = textline.substr(valueStart+1);

      // Create the XML element
      XElement elem(name,
                    XAttribute("class", label),
                    XText(value)
                    );

      // Add it to the quality metrics
      qlt.Append(elem);
  }


};
}
}

// Software Guide : BeginLatex
// Finally \code{OTB\_APPLICATION\_EXPORT} is called.
// Software Guide : EndLatex
//  Software Guide :BeginCodeSnippet
OTB_APPLICATION_EXPORT(otb::Wrapper::XMLStatistics)
//  Software Guide :EndCodeSnippet


