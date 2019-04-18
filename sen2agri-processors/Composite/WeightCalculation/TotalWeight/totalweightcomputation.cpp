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
 
#include "totalweightcomputation.h"

TotalWeightComputation::TotalWeightComputation()
{
    m_fWeightOnSensor = -1;
    m_fWeightOnDateMin = 0.5;
    m_res = -1;
}


void TotalWeightComputation::SetMissionName(std::string &missionName)
{
    if(missionName.find("SENTINEL2") == 0)  {
        m_sensorType = S2;
    } else {
        m_sensorType = L8;
    }
}

void TotalWeightComputation::SetDates(std::string& L2ADate, std::string& L3ADate)
{
    //std::cout << "L2ADate: " << L2ADate << std::endl;
    //std::cout << "L3ADate: " << L3ADate << std::endl;

    struct tm tmTime = {};
    strptime(L2ADate.c_str(), "%Y%m%d", &tmTime);
    time_t ttL2ATime = mktime(&tmTime);
    strptime(L3ADate.c_str(), "%Y%m%d", &tmTime);
    time_t ttL3ATime = mktime(&tmTime);
    double seconds = fabs(difftime(ttL2ATime, ttL3ATime));
    // compute the time difference as the number of days
    m_nDaysTimeInterval = (int)(seconds / (3600 *24));
}


void TotalWeightComputation::SetHalfSynthesisPeriodAsDays(int deltaMax)
{
    m_nDeltaMax = deltaMax;
}

void TotalWeightComputation::SetWeightOnDateMin(float fMinWeight)
{
    m_fWeightOnDateMin = fMinWeight;
}

void TotalWeightComputation::SetAotWeightFile(std::string &aotWeightFileName)
{
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(aotWeightFileName);
    m_inputReaderAot = reader;
}

void TotalWeightComputation::SetCloudsWeightFile(std::string &cloudsWeightFileName)
{
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(cloudsWeightFileName);
    m_inputReaderCld = reader;
}

void TotalWeightComputation::SetTotalWeightOutputFileName(std::string &outFileName)
{
    m_strOutFileName = outFileName;
}

TotalWeightComputation::OutImageSource::Pointer TotalWeightComputation::GetOutputImageSource()
{
    BuildOutputImageSource();
    return (OutImageSource::Pointer)m_filter;
}

void TotalWeightComputation::BuildOutputImageSource()
{
    ComputeTotalWeight();
}

void TotalWeightComputation::ComputeWeightOnSensor()
{
    if(m_fWeightOnSensor == -1) {
        switch(m_sensorType){
            case S2:
                m_fWeightOnSensor = 1;
                break;
            case L8:
            default:
                m_fWeightOnSensor = 0.33;
                break;
        }
    }
}

void TotalWeightComputation::ComputeWeightOnDate()
{
    //std::cout << "ComputeWeightOnDate: " << std::endl;
    //std::cout << "\tm_nDaysTimeInterval: " << m_nDaysTimeInterval << std::endl;
    //std::cout << "\tm_nDeltaMax : " << m_nDeltaMax << std::endl;
    //std::cout << "\tm_fWeightOnDateMin : " << m_fWeightOnDateMin << std::endl;
    m_fWeightOnDate = 1 - (abs(m_nDaysTimeInterval)/m_nDeltaMax) * (1-m_fWeightOnDateMin);
}

void TotalWeightComputation::ComputeTotalWeight()
{
    ComputeWeightOnSensor();
    ComputeWeightOnDate();

    m_filter = FilterType::New();
    m_filter->GetFunctor().SetFixedWeight(m_fWeightOnSensor, m_fWeightOnDate);
    ImageType::Pointer imgAot = m_inputReaderAot->GetOutput();
    ImageType::Pointer imgCld = m_inputReaderCld->GetOutput();
    imgAot->UpdateOutputInformation();
    imgCld->UpdateOutputInformation();
    ImageType::SpacingType spacingAot = imgAot->GetSpacing();
    ImageType::SpacingType spacingCld = imgCld->GetSpacing();

    ImageType::PointType originAot = imgAot->GetOrigin();
    ImageType::PointType originCld = imgCld->GetOrigin();
    // normally, the AOT weight should have the same spacing as the clouds weight
    if((spacingAot[0] != spacingCld[0]) || (spacingAot[1] != spacingCld[1]) ||
       (originAot[0] != originCld[0]) || (originAot[1] != originCld[1])) {
        float fMultiplicationFactor = ((float)spacingAot[0])/spacingCld[0];
        //force the origin and the resolution to the one from cloud image
        imgAot = m_AotResampler.getResampler(imgAot, fMultiplicationFactor, originCld)->GetOutput();
    }

    m_filter->SetInput1(imgAot);
    m_filter->SetInput2(imgCld);

    //m_filter->SetDirectionTolerance(5);
    //m_filter->SetCoordinateTolerance(5);
    //CheckTolerance();
}

typedef double SpacePrecisionType;

void TotalWeightComputation::CheckTolerance()
{
    double m_DirectionTolerance = 0.001;
    double m_CoordinateTolerance = 0.001;

    m_inputReaderAot->UpdateOutputInformation();
    m_inputReaderCld->UpdateOutputInformation();

    ImageType *inputPtr1=
      dynamic_cast< ImageType * >( m_inputReaderAot->GetOutput() );

    ImageType *inputPtrN =
      dynamic_cast< ImageType * >( m_inputReaderCld->GetOutput() );

    // tolerance for origin and spacing depends on the size of pixel
    // tolerance for directions a fraction of the unit cube.
    const SpacePrecisionType coordinateTol
      = m_CoordinateTolerance * inputPtr1->GetSpacing()[0]; // use first dimension spacing

    /*vnl_vector<double> vect1 = inputPtr1->GetOrigin().GetVnlVector();
    double val11 = vect1.get(0);
    double val12 = vect1.get(1);
    vnl_vector<double> vect2 = inputPtrN->GetOrigin().GetVnlVector();
    double val21 = vect2.get(0);
    double val22 = vect2.get(1);

    double diff1 = fabs(val11-val21);
    double diff2 = fabs(val12-val22);

    vnl_vector<double> vect3 = inputPtr1->GetSpacing().GetVnlVector();
    double val31 = vect3.get(0);
    double val32 = vect3.get(1);

    vnl_vector<double> vect4 = inputPtrN->GetSpacing().GetVnlVector();
    double val41 = vect4.get(0);
    double val42 = vect4.get(1);

    double diff3 = fabs(val31-val41);
    double diff4 = fabs(val32-val42);*/


    //vnl_vector<SpacingValueType> vect1 = inputPtr1->GetOrigin().GetVnlVector();
    //vnl_vector<SpacingValueType> vect2 = inputPtrN->GetOrigin().GetVnlVector();

    if ( !inputPtr1->GetOrigin().GetVnlVector().is_equal(inputPtrN->GetOrigin().GetVnlVector(), coordinateTol) ||
         !inputPtr1->GetSpacing().GetVnlVector().is_equal(inputPtrN->GetSpacing().GetVnlVector(), coordinateTol) ||
         !inputPtr1->GetDirection().GetVnlMatrix().as_ref().is_equal(inputPtrN->GetDirection().GetVnlMatrix(), m_DirectionTolerance) )
      {
      std::ostringstream originString, spacingString, directionString;
      if ( !inputPtr1->GetOrigin().GetVnlVector().is_equal(inputPtrN->GetOrigin().GetVnlVector(), coordinateTol) )
        {
        originString.setf( std::ios::scientific );
        originString.precision( 7 );
        originString << "InputImage Origin: " << inputPtr1->GetOrigin()
                     << ", InputImage" << " Origin: " << inputPtrN->GetOrigin() << std::endl;
        originString << "\tTolerance: " << coordinateTol << std::endl;
        }
      if ( !inputPtr1->GetSpacing().GetVnlVector().is_equal(inputPtrN->GetSpacing().GetVnlVector(), coordinateTol) )
        {
        spacingString.setf( std::ios::scientific );
        spacingString.precision( 7 );
        spacingString << "InputImage Spacing: " << inputPtr1->GetSpacing()
                      << ", InputImage"  << " Spacing: " << inputPtrN->GetSpacing() << std::endl;
        spacingString << "\tTolerance: " << coordinateTol << std::endl;
        }
      if ( !inputPtr1->GetDirection().GetVnlMatrix().as_ref().is_equal(inputPtrN->GetDirection().GetVnlMatrix(), m_DirectionTolerance) )
        {
        directionString.setf( std::ios::scientific );
        directionString.precision( 7 );
        directionString << "InputImage Direction: " << inputPtr1->GetDirection()
                        << ", InputImage" << " Direction: " << inputPtrN->GetDirection() << std::endl;
        directionString << "\tTolerance: " << m_DirectionTolerance << std::endl;
        }
      itkExceptionMacro(<< "Inputs do not occupy the same physical space! "
                        << std::endl
                        << originString.str() << spacingString.str()
                        << directionString.str() );
      }
}

void TotalWeightComputation::WriteToOutputFile()
{
    if(!m_strOutFileName.empty())
    {
        WriterType::Pointer writer;
        writer = WriterType::New();
        writer->SetFileName(m_strOutFileName);
        writer->SetInput(GetOutputImageSource()->GetOutput());
        try
        {
            writer->Update();
        }
        catch (itk::ExceptionObject& err)
        {
            std::cout << "ExceptionObject caught !" << std::endl;
            std::cout << err << std::endl;
            itkExceptionMacro("Error writing output");
        }
    }
}
