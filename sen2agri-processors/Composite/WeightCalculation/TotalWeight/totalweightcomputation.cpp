#include "totalweightcomputation.h"

TotalWeightComputation::TotalWeightComputation()
{
    m_fWeightOnSensor = -1;
    m_fWeightOnDateMin = 0.5;
}


void TotalWeightComputation::SetMissionName(std::string &missionName)
{
    if(missionName == "SENTINEL-2A") {
        m_sensorType = S2A;
    } else if(missionName == "SENTINEL-2B") {
        m_sensorType = S2B;
    } else {
        m_sensorType = L8;
    }
}

void TotalWeightComputation::SetWeightOnSensor(float fWeight)
{
    m_fWeightOnSensor = fWeight;
}

void TotalWeightComputation::SetDates(std::string& L2ADate, std::string& L3ADate)
{
    // strptime does not handles correctly the strings in the format YYYYMMDD
    // when MM or DD start with 0
//    std::string formattedL2ADate = L2ADate.substr(0,4) + "-" +
//                                   L2ADate.substr(4,2) + "-" +
//                                   L2ADate.substr(6,2);
//    std::string formattedL3ADate = L3ADate.substr(0,4) + "-" +
//                                   L3ADate.substr(4,2) + "-" +
//                                   L3ADate.substr(6,2);
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
    m_inputReader1 = reader;
}

void TotalWeightComputation::SetCloudsWeightFile(std::string &cloudsWeightFileName)
{
    ReaderType::Pointer reader = ReaderType::New();
    reader->SetFileName(cloudsWeightFileName);
    m_inputReader2 = reader;
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
            case S2A:
                m_fWeightOnSensor = 1;
                break;
            case S2B:
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
    m_fWeightOnDate = 1 - (abs(m_nDaysTimeInterval)/m_nDeltaMax) * (1-m_fWeightOnDateMin);
}

void TotalWeightComputation::ComputeTotalWeight()
{
    ComputeWeightOnSensor();
    ComputeWeightOnDate();

    m_filter = FilterType::New();
    m_filter->GetFunctor().SetFixedWeight(m_fWeightOnSensor, m_fWeightOnDate);
    m_filter->SetInput1(m_inputReader1->GetOutput());
    m_filter->SetInput2(m_inputReader2->GetOutput());
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
