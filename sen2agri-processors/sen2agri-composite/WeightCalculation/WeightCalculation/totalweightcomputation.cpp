#include "totalweightcomputation.h"

TotalWeightComputation::TotalWeightComputation()
{
    m_fWeightOnSensor = -1;
    m_fWeightOnDateMin = 0.5;
}


void TotalWeightComputation::SetInputProductName(std::string &inputProductName)
{
    std::string strPrefix;
    if(inputProductName.size() > 3) {
        strPrefix = inputProductName.substr(3);
    }
    if(strPrefix == "S2A") {
        m_sensorType = S2A;
    } else if(strPrefix == "S2B") {
        m_sensorType = S2B;
    } else {
        m_sensorType = L8;
    }
}

void TotalWeightComputation::SetWeightOnSensor(float fWeight)
{
    m_fWeightOnSensor = fWeight;
}

void TotalWeightComputation::SetL2ADateAsDays(int nL2ADays)
{
    m_nL2ADays = nL2ADays;
}

void TotalWeightComputation::SetL3ADateAsDays(int nL3ADays)
{
    m_nL3ADays = nL3ADays;
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
    return (OutImageSource::Pointer)m_filter;
}

void TotalWeightComputation::Update()
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
    m_fWeightOnDate = 1 - (abs(m_nL2ADays - m_nL3ADays)/m_nDeltaMax) * (1-m_fWeightOnDateMin);
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
        writer->SetInput(m_filter->GetOutput());
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
