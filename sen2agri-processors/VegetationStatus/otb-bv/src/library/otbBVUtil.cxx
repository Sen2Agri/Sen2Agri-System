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
#include <boost/algorithm/string.hpp>
#include "itkMacro.h"

namespace otb
{
size_t countColumns(std::string fileName)
{
  std::ifstream ifile(fileName.c_str());
  std::string line;
  size_t nbSpaces = 0;
  if (ifile.is_open())
    {
    getline(ifile,line);
    ifile.close();
    boost::trim(line);
    auto found = line.find(' ');
    while(found!=std::string::npos)
      {
      ++nbSpaces;
      found = line.find(' ', found+1);
      }
    return nbSpaces+1;
    }
  else
    {
    itkGenericExceptionMacro(<< "Could not open file " << fileName);
    }

}
}

