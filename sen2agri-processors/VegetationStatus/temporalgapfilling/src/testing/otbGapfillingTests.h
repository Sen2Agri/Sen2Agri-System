/*=========================================================================

  Program:   gapfilling
  Language:  C++

  Copyright (c) Jordi Inglada. All rights reserved.

  See gapfilling-copyright.txt for details.

  This software is distributed WITHOUT ANY WARRANTY; without even
  the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
  PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "itkMacro.h"

#include "itkVariableLengthVector.h"
#include "gapfilling.h"
#include "phenoFunctions.h"
#include <tuple>

using ValueType = double;
using PixelType = itk::VariableLengthVector<ValueType>;
using VectorType = std::vector<ValueType>;
using TestDataType = std::vector<std::tuple<VectorType, VectorType, VectorType, VectorType, VectorType> >;
using MCTestDataType = std::vector<std::tuple<VectorType, VectorType, VectorType> >; 
using MCTestOutDatesDataType = std::vector<std::tuple<VectorType, VectorType, VectorType, VectorType> >; 
