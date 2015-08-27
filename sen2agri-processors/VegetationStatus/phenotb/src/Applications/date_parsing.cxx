/*=========================================================================

  Program:   phenotb
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See phenotb-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkMacro.h"
#include "dateUtils.h"
#include <iostream>

int main(int argc, char* argv[])
{
  if(argc!=2)
    itkGenericExceptionMacro(<< "Usage: " << argv[0] << " filename\n");
  auto vdates(pheno::parse_date_file(std::string(argv[1])));
  std::cout << vdates.size() << " dates read.\n";
  for(auto & vdate : vdates)
    {
    auto dd = vdate;
    std::cout << dd.tm_year << "\t\t" << dd.tm_mon << "\t\t" << dd.tm_mday;
    std::cout << "\t\t" << pheno::doy(dd);
    std::cout << std::endl;
    }
  return 0;
}
