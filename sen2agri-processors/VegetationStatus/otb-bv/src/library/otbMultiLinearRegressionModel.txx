/*=========================================================================
  Program:   otb-bv
  Language:  C++

  Copyright (c) CESBIO. All rights reserved.

  See otb-bv-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <fstream>
#include <iomanip>

namespace otb{
template <typename PrecisionType>
void  MultiLinearRegressionModel<PrecisionType>::multi_linear_fit()
{
  auto n = m_x.size();
  auto m = m_x[0].size()+1;
  auto X = gsl_matrix_alloc (n, m);
  auto y = gsl_vector_alloc (n);
  auto w = gsl_vector_alloc (n);
  auto c = gsl_vector_alloc (m);
  auto cov = gsl_matrix_alloc (m, m);
  for (size_t i = 0; i < n; i++)
    {
    gsl_matrix_set (X, i, 0, 1.0);
    for(size_t j=0; j<m-1; j++)
      gsl_matrix_set(X, i, j+1, m_x[i][j]);
    gsl_vector_set (y, i, m_y[i]);
    if(m_weights)
      {
      gsl_vector_set (w, i, 1.0/(m_w[i]*m_w[i]));
      }
    else
      {
      gsl_vector_set (w, i, 1.0);
      }
    }
  gsl_multifit_linear_workspace * work 
    = gsl_multifit_linear_alloc (n, m);
  double chisq{0};
  gsl_multifit_wlinear(X, w, y, c, cov, &chisq, work);
  gsl_multifit_linear_free(work);

  m_model = VectorType{};
  for(size_t j=0; j<m; j++)
    m_model.push_back(gsl_vector_get(c,j));
}

template <typename PrecisionType>
void  MultiLinearRegressionModel<PrecisionType>::Save(const std::string & 
                                                      filename, 
                                                      const std::string & name){
  if(name==""){};//ugly silenting of unused variable
  std::ofstream model_file;
  try
    {
    model_file.open(filename.c_str(), std::ofstream::out);
    }
  catch(...)
    {
    itkGenericExceptionMacro(<< "Could not open file " << filename);
    }
  model_file << "# Multilinear regression model\n";
  model_file << std::setprecision(10);
  for(auto& coef: m_model)
    model_file << coef << "\n";
  model_file.close();
}
template <typename PrecisionType>
void  MultiLinearRegressionModel<PrecisionType>::Load(const std::string & 
                                                      filename, 
                                                      const std::string & name){
  if(name==""){};//ugly silenting of unused variable  
  std::ifstream model_file;
  try
    {
    model_file.open(filename.c_str());
    }
  catch(...)
    {
    itkGenericExceptionMacro(<< "Could not open file " << filename);
    }
  m_model.clear();
  std::string line;
  std::getline(model_file, line); //skip header line
  while(std::getline(model_file, line))
    {
    if(line.size() > 1)
      {
      std::istringstream ss(line);
      PrecisionType value;
      ss >> value;
      m_model.push_back(value);
      }
    else
      itkGenericExceptionMacro(<< "Bad format in model file " << filename 
                               << "\n" << line << "\n");
    }
  model_file.close();
}

template <typename PrecisionType>
bool MultiLinearRegressionModel<PrecisionType>::CanReadFile(const std::string & 
                                                            file)
{
  std::ifstream ifs;
  ifs.open(file.c_str());

  if (!ifs)
    {
    std::cerr << "Could not read file " << file << std::endl;
    return false;
    }

  while (!ifs.eof())
    {
    std::string line;
    std::getline(ifs, line);

    if (line.find("Multilinear") != std::string::npos)
      {
      return true;
      }
    }
  ifs.close();
  return false;
}

template <typename PrecisionType>
bool MultiLinearRegressionModel<PrecisionType>::CanWriteFile(const std::string & 
                                                             itkNotUsed(file))
{
  return false;
}

}//namespace otb
