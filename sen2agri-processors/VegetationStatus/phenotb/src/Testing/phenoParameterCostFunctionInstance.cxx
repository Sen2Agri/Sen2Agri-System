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
#include "phenoFunctions.h"

int phenoParameterCostFunctionInstance(int argc, char * argv[])
{
  if(argc>1)
    {
    for(auto i=0; i<argc; ++i)
      std::cout << i << " --> " << argv[i] << std::endl;
    return EXIT_FAILURE;
    }

  const unsigned int params{6};
  auto phF(pheno::sigmoid::F<pheno::VectorType>);
  const unsigned int nbDates{12};
  pheno::VectorType t(nbDates);
  for(size_t i=0; i<nbDates; ++i)
    t[i] = i;
  pheno::VectorType xt(params);
  pheno::VectorType y(phF(t,xt));
  // Set up a pheno::ParameterCostFunction compute object
  pheno::ParameterCostFunction f{params, nbDates, y, t, phF};
  return EXIT_SUCCESS;
}
