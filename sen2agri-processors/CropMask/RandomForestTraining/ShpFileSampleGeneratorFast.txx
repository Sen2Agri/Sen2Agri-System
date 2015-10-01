/*
 * ShpFileSampleGeneratorFast.cxx
 *
 *  Created on: 19/1/2015
 *      Author: silvia
 */


#include "otbVectorImage.h"
#include "otbVectorData.h"
#include <iostream>

template <class TImage, class TVectorData>
ShpFileSampleGeneratorFast<TImage, TVectorData>::ShpFileSampleGeneratorFast ( ) :
_NumberOfClasses(0), _NumberOfCropSamples(0), _NumberOfNoCropSamples(0), _MaximumTrainingSamples(0), _seed(0), _LimitationofMaximumTrainingSamples(0), _ClassKey("CODE"), _CropClassKey("CROP"), _LabelClassKey("LC")
{

	_Samples = ListSampleType:: New();
	_CoordinatesList = CoordinateList:: New();

	//Para que sirve?
	this->SetNumberOfRequiredInputs(2);
	this->SetNumberOfRequiredOutputs(8);

	// Register the outputs
	this->itk::ProcessObject::SetNthOutput(0, this->MakeOutput(0).GetPointer());
	this->itk::ProcessObject::SetNthOutput(1, this->MakeOutput(1).GetPointer());
	this->itk::ProcessObject::SetNthOutput(2, this->MakeOutput(2).GetPointer());
	this->itk::ProcessObject::SetNthOutput(3, this->MakeOutput(3).GetPointer());
	this->itk::ProcessObject::SetNthOutput(4, this->MakeOutput(4).GetPointer());
	this->itk::ProcessObject::SetNthOutput(5, this->MakeOutput(5).GetPointer());
	this->itk::ProcessObject::SetNthOutput(6, this->MakeOutput(6).GetPointer());
	this->itk::ProcessObject::SetNthOutput(7, this->MakeOutput(7).GetPointer());

};


template <class TImage, class TVectorData>
typename ShpFileSampleGeneratorFast<TImage, TVectorData>::DataObjectPointer
ShpFileSampleGeneratorFast<TImage, TVectorData>::MakeOutput(unsigned int idx)
{
	DataObjectPointer output;
	switch (idx)
	{
		case 0:
			output = static_cast<itk::DataObject*>(ListSampleType::New().GetPointer());
			break;
		case 1:
			output = static_cast<itk::DataObject*>(ListLabelType::New().GetPointer());
			break;
		case 2:
			output = static_cast<itk::DataObject*>(ListSampleType::New().GetPointer());
			break;
		case 3:
			output = static_cast<itk::DataObject*>(ListLabelType::New().GetPointer());
			break;
		case 4:
			output = static_cast<itk::DataObject*>(ListLabelType::New().GetPointer());
			break;
		case 5:
			output = static_cast<itk::DataObject*>(ListLabelType::New().GetPointer());
			break;
		case 6:
			output = static_cast<itk::DataObject*>(CoordinateList::New().GetPointer());
			break;

		case 7:
			output = static_cast<itk::DataObject*>(CoordinateList::New().GetPointer());
			break;

		default:
			output = static_cast<itk::DataObject*>(CoordinateList::New().GetPointer());
			break;
	}
	return output;
}


template <class TImage, class TVectorData>
void ShpFileSampleGeneratorFast<TImage, TVectorData>::SetInputImage(TImage * InputImage)
{

	this->ProcessObject::SetNthInput( 0, const_cast<TImage *>(InputImage) );

};

template <class TImage, class TVectorData>
const TImage* ShpFileSampleGeneratorFast<TImage, TVectorData>::GetInput() const
{
	if (this->GetNumberOfInputs() < 1)
	{
		return 0;
	}
  return static_cast<const TImage *>(this->ProcessObject::GetInput(0));
}


template <class TImage, class TVectorData>
void ShpFileSampleGeneratorFast<TImage, TVectorData>::SetInputVectorData(const TVectorData * VectorDataInput)
{

	this->ProcessObject::SetNthInput( 1, const_cast<TVectorData *>(VectorDataInput) );
};


template <class TImage, class TVectorData>
const TVectorData* ShpFileSampleGeneratorFast<TImage, TVectorData>::GetInputVectorData() const
{

	if (this->GetNumberOfInputs() < 2)
	{
		return 0;
	}
  return static_cast<const TVectorData *>(this->ProcessObject::GetInput(1));
}



template <class TImage, class TVectorData>
double ShpFileSampleGeneratorFast<TImage, TVectorData>::GetPolygonAreaPixels(DataNodeType*  datanode, TImage* image)
{
	auto area = 0;
	typename TImage::RegionType imageLargestRegion = image->GetLargestPossibleRegion();

	PolygonPointerType exteriorRing = datanode->GetPolygonExteriorRing();

	typename  TImage::RegionType polygonRegion =
	otb::TransformPhysicalRegionToIndexRegion(exteriorRing->GetBoundingRegion(),image);
	 const bool hasIntersection = polygonRegion.Crop(imageLargestRegion);
	if (hasIntersection)
     	{

	 image->SetRequestedRegion(polygonRegion);
	 image->PropagateRequestedRegion();
	 image->UpdateOutputData();

	 typedef itk::ImageRegionConstIteratorWithIndex<TImage> IteratorType;
	IteratorType it(image, polygonRegion);
	for (it.GoToBegin(); !it.IsAtEnd(); ++it)
	{
		itk::ContinuousIndex<double, 2> point;
		image->TransformIndexToPhysicalPoint(it.GetIndex(), point);
		bool isInterior = false;
	   if ( exteriorRing->IsInside(point) )
	   {
		PolygonListPointerType interiorRings = datanode->GetPolygonInteriorRings();
		for (typename PolygonListType::Iterator interiorRing = interiorRings->Begin();interiorRing != interiorRings->End();++interiorRing)
		{
			if ( interiorRing.Get()->IsInside(point) )
			{
	  			isInterior = true;	  
               		}
		}

		bool MaskPixel = true;
		auto ii=0;
		while ( ii < it.Get().GetSize()-1 && MaskPixel )
		{
			if(it.Get()[ii]!=it.Get()[ii+1])
			{
				MaskPixel = false;
			}
                    ii++;
		}

		//We need to verify that it is not a mask point

		if(!isInterior && !MaskPixel){
			++area;						 
		}
	     }
	         
	     }
         
	}

  return area;
}

template <class TImage, class TVectorData>
void ShpFileSampleGeneratorFast<TImage, TVectorData>::SetSeed(unsigned int seed)
{
	_seed = seed;
}


template <class TImage, class TVectorData>
void ShpFileSampleGeneratorFast<TImage, TVectorData>::SetLimitationofMaximumTrainingSamples( unsigned int LimitationofMaximumTrainingSamples)
{
	_LimitationofMaximumTrainingSamples = LimitationofMaximumTrainingSamples;
}


template <class TImage, class TVectorData>
void ShpFileSampleGeneratorFast<TImage, TVectorData>::GenerateClassStatistics()
{
	_ClassesSize.clear();

	TImage* image = const_cast<TImage*> (this->GetInput());
	const TVectorData* vectorData = const_cast<TVectorData*>(this->GetInputVectorData());


	typename TImage::RegionType imageLargestRegion = image->GetLargestPossibleRegion();

	_Samples->Clear();
	_Samples->SetMeasurementVectorSize(image->GetNumberOfComponentsPerPixel());

	_CoordinatesList->Clear();
	_CoordinatesList->SetMeasurementVectorSize(2);

	// Compute cumulative area of all polygons of each class
	TreeIteratorType itVector(vectorData->GetDataTree());
	for (itVector.GoToBegin(); !itVector.IsAtEnd(); ++itVector)
	{
		DataNodeType* datanode = itVector.Get();
		if (datanode->IsPolygonFeature())
		{

		  PolygonPointerType exteriorRing = datanode->GetPolygonExteriorRing();
	      typename  TImage::RegionType polygonRegion = otb::TransformPhysicalRegionToIndexRegion(exteriorRing->GetBoundingRegion(),image);

		  const bool hasIntersection = polygonRegion.Crop(imageLargestRegion);
		  if (hasIntersection)
		  {
			  double area = GetPolygonAreaPixels(datanode, image);
			  if (area > 0)
			  {
				_ClassesSize[datanode->GetFieldAsInt(_ClassKey)] += std::floor(area);

				if (datanode->GetFieldAsInt(_CropClassKey) == 0)
				{
						_NumberOfNoCropSamples += std::floor(area);
				}
				else
				{
						_NumberOfCropSamples += std::floor(area);
				}

				ClassLabelType label = datanode->GetFieldAsInt(_ClassKey);
				if ( _ClassesLabels.find(label) == _ClassesLabels.end() )
				{
									// not found

					std::string labelName = datanode->GetFieldAsString(_LabelClassKey);
					_ClassesLabels.insert ( std::pair<ClassLabelType, std::string>(label,labelName) );
				}
					bool IsCrop = (datanode->GetFieldAsInt(_CropClassKey) == 1);
					_ClassesCrop.insert(std::pair<ClassLabelType, bool>(label,IsCrop));

					image->SetRequestedRegion(polygonRegion);
					image->PropagateRequestedRegion();
					image->UpdateOutputData();

					typedef itk::ImageRegionConstIteratorWithIndex<TImage> IteratorType;
					IteratorType it(image, polygonRegion);

					unsigned int Nsamples = _Samples->Size();

					for (it.GoToBegin(); !it.IsAtEnd(); ++it)
					{

						itk::ContinuousIndex<double, 2> point;
						image->TransformIndexToPhysicalPoint(it.GetIndex(), point);
						bool isInterior = false;
						if ( exteriorRing->IsInside(point))
						{
 							PolygonListPointerType interiorRings = datanode->GetPolygonInteriorRings();
						
							for (typename PolygonListType::Iterator interiorRing = interiorRings->Begin();interiorRing != interiorRings->End();
							++interiorRing)
							{
								if ( interiorRing.Get()->IsInside(point)  )
								{
									isInterior = true;
								}
							}

							bool MaskPixel = true;
							auto ii=0;
							while ( ii < it.Get().GetSize()-1 && MaskPixel )
							{
								if(it.Get()[ii]!=it.Get()[ii+1])
								{
									MaskPixel = false;
								}
                    						ii++;
							}


							if(!isInterior && !MaskPixel){
								_Samples->PushBack(it.Get());
						 		_CoordinateSamples[label].push_back(Nsamples);
						 		Nsamples ++;

								CoordinateType coor;
						  		coor[0] = it.GetIndex()[0];
						  		coor[1] = it.GetIndex()[1];

						  		_CoordinatesList ->PushBack(coor);


							}
						}
					}



			}
		  }
		}
	}
	_NumberOfClasses = _ClassesSize.size();

	//float alpha = 0.25;
	_MaximumTrainingSamples = std::min(_NumberOfCropSamples,_NumberOfNoCropSamples);
	_MaximumTrainingSamples = std::min(_MaximumTrainingSamples, _LimitationofMaximumTrainingSamples);
}





template <class TImage, class TVectorData>
void ShpFileSampleGeneratorFast<TImage, TVectorData>::GenerateSampleLists()
{

	TImage* image = const_cast<TImage*> (this->GetInput());

	typename ListSampleType::Pointer trainingListSample   = this->GetTrainingListSample();
	typename ListLabelType::Pointer  trainingListLabel    = this->GetTrainingListLabel();
	typename ListLabelType::Pointer  trainingListLabelCrop    = this->GetTrainingListLabelCrop();

	typename ListSampleType::Pointer validationListSample = this->GetValidationListSample();
	typename ListLabelType::Pointer  validationListLabel  = this->GetValidationListLabel();
	typename ListLabelType::Pointer  validationListLabelCrop  = this->GetValidationListLabelCrop();


	typename CoordinateList::Pointer  trainingCoordinatesList  = this->GetTrainingCoordinatesList();
	typename CoordinateList::Pointer  validationCoordinatesList  = this->GetValidationCoordinatesList();


	trainingListSample->Clear();
	trainingListLabel->Clear();
	trainingListLabelCrop->Clear();
	validationListSample->Clear();
	validationListLabel->Clear();
	validationListLabelCrop->Clear();
	trainingCoordinatesList ->Clear();
	validationCoordinatesList ->Clear();

	trainingListSample->SetMeasurementVectorSize(image->GetNumberOfComponentsPerPixel());
	trainingListLabel->SetMeasurementVectorSize(1);
	trainingListLabelCrop->SetMeasurementVectorSize(1);
	validationListSample->SetMeasurementVectorSize(image->GetNumberOfComponentsPerPixel());
	validationListLabel->SetMeasurementVectorSize(1);
	validationListLabelCrop->SetMeasurementVectorSize(1);
	trainingCoordinatesList->SetMeasurementVectorSize(2);
	validationCoordinatesList->SetMeasurementVectorSize(2);


	std::map<ClassLabelType, unsigned int>::const_iterator itmap;

	for (itmap = _ClassesSize.begin(); itmap != _ClassesSize.end(); ++itmap)
	{
		bool IsCrop = _ClassesCrop[itmap->first];
		// We need to split the samples of the specific class : Training and Testing
		// The proportion NumberSamplesMais/_NumberOfCropSamples is kept for the number of training samples
		auto NumberOfTrainingSamples = 0;
		if(IsCrop)
		{
			NumberOfTrainingSamples = std::floor(_MaximumTrainingSamples*itmap->second)/_NumberOfCropSamples;
		}
		else
		{
			NumberOfTrainingSamples = std::floor(_MaximumTrainingSamples*itmap->second)/_NumberOfNoCropSamples;
		}

		std::vector<unsigned int> SampleSequenceID;
		//Initialising Sequence
		for( unsigned int i= 0; i <itmap->second; ++i)
		{
			SampleSequenceID.push_back(i);
		}

		//auto seed = unsigned(std::time(0));
		//auto seed = 1508722459;
		std::srand(_seed);
		//std::srand(unsigned(std::time(0)));
		std::random_shuffle(SampleSequenceID.begin(),SampleSequenceID.end());

		std::cout<<itmap->first<<" NumberOfTrainingSamples  " <<NumberOfTrainingSamples << std::endl;
		for (auto i=0;i<SampleSequenceID.size();i++)
		{

			unsigned int pos = SampleSequenceID.at(i);
			unsigned int CurrentCoordinateSample= _CoordinateSamples[itmap->first].at(pos);

		//	std::cout<<"Nsamples"<< _Samples->Size()  << std::endl;
		//	std::cout<<"pos"<< pos << "CurrentCoordinateSample "<<CurrentCoordinateSample  << std::endl;



			if (i < NumberOfTrainingSamples )
			{
				trainingListSample->PushBack(_Samples->GetMeasurementVector(CurrentCoordinateSample));
				trainingListLabel->PushBack(itmap->first);
				trainingListLabelCrop->PushBack((LabelType) IsCrop);
				trainingCoordinatesList->PushBack(_CoordinatesList->GetMeasurementVector(CurrentCoordinateSample));
			}
			else{
		       	validationListSample->PushBack(_Samples->GetMeasurementVector(CurrentCoordinateSample));
		        validationListLabel->PushBack(itmap->first);
		        validationListLabelCrop->PushBack((LabelType) IsCrop);
			validationCoordinatesList->PushBack(_CoordinatesList->GetMeasurementVector(CurrentCoordinateSample));

			}

		}



	}//Going on all the classes

}//End


template <class TImage, class TVectorData>
int ShpFileSampleGeneratorFast<TImage, TVectorData>::GetNumberOfClasses ( )
{
  return (this->_NumberOfClasses);
};


template <class TImage, class TVectorData>
std::map< int, std::string>*  ShpFileSampleGeneratorFast<TImage, TVectorData>::GetInfoLabelsOfClasses ( )
{
  return (&(this->_ClassesLabels));
};


template <class TImage, class TVectorData>
std::map< int, unsigned int>*  ShpFileSampleGeneratorFast<TImage, TVectorData>::GetInfoSamplesOfClasses ( )
{
  return (&(this->_ClassesSize));
};


template <class TImage, class TVectorData>
void ShpFileSampleGeneratorFast<TImage, TVectorData>::PrintInformation( )
{
	 std::cout<< " The total number of samples for the crop type is : " << 	_NumberOfCropSamples<<std::endl;
	 std::cout<< " The total number of samples for the no crop type is : " << 	_NumberOfNoCropSamples<<std::endl;

	 std::cout<< " The maximum number of samples for both classes is : " << _MaximumTrainingSamples <<std::endl;


	 std::cout<< " The description of both classes is : " <<std::endl;
	 std::map<ClassLabelType, unsigned int>::const_iterator itmap;

	 for (itmap = _ClassesSize.begin(); itmap != _ClassesSize.end(); ++itmap)
	 {
		 std::cout<< " 		Class "<< _ClassesLabels[itmap->first] << " with Label  " << itmap->first << " has " << itmap->second << " Samples"<<std::endl;
	 }

};

// Get the Training ListSample
template <class TImage, class TVectorData>
typename ShpFileSampleGeneratorFast<TImage, TVectorData>::ListSampleType*
ShpFileSampleGeneratorFast<TImage, TVectorData>
::GetTrainingListSample()
{
  return dynamic_cast<ListSampleType*>(this->itk::ProcessObject::GetOutput(0));
}
// Get the Training label ListSample
template <class TImage, class TVectorData>
typename ShpFileSampleGeneratorFast<TImage, TVectorData>::ListLabelType*
ShpFileSampleGeneratorFast<TImage, TVectorData>
::GetTrainingListLabel()
{
  return dynamic_cast<ListLabelType*>(this->itk::ProcessObject::GetOutput(1));
}

// Get the Training label ListSample
template <class TImage, class TVectorData>
typename ShpFileSampleGeneratorFast<TImage, TVectorData>::ListLabelType*
ShpFileSampleGeneratorFast<TImage, TVectorData>
::GetTrainingListLabelCrop()
{
  return dynamic_cast<ListLabelType*>(this->itk::ProcessObject::GetOutput(4));
}


// Get the validation ListSample
template <class TImage, class TVectorData>
typename ShpFileSampleGeneratorFast<TImage, TVectorData>::ListSampleType*
ShpFileSampleGeneratorFast<TImage, TVectorData>
::GetValidationListSample()
{
  return dynamic_cast<ListSampleType*>(this->itk::ProcessObject::GetOutput(2));
}


// Get the validation label ListSample
template <class TImage, class TVectorData>
typename ShpFileSampleGeneratorFast<TImage, TVectorData>::ListLabelType*
ShpFileSampleGeneratorFast<TImage, TVectorData>
::GetValidationListLabel()
{
  return dynamic_cast<ListLabelType*>(this->itk::ProcessObject::GetOutput(3));
}



// Get the validation label ListSampleCrop
template <class TImage, class TVectorData>
typename ShpFileSampleGeneratorFast<TImage, TVectorData>::ListLabelType*
ShpFileSampleGeneratorFast<TImage, TVectorData>
::GetValidationListLabelCrop()
{
  return dynamic_cast<ListLabelType*>(this->itk::ProcessObject::GetOutput(5));
}


// Get the validation label ListSampleCrop
template <class TImage, class TVectorData>
typename ShpFileSampleGeneratorFast<TImage, TVectorData>::CoordinateList*
ShpFileSampleGeneratorFast<TImage, TVectorData>
::GetTrainingCoordinatesList()
{
  return dynamic_cast<CoordinateList*>(this->itk::ProcessObject::GetOutput(6));
}



// Get the validation label ListSampleCrop
template <class TImage, class TVectorData>
typename ShpFileSampleGeneratorFast<TImage, TVectorData>::CoordinateList*
ShpFileSampleGeneratorFast<TImage, TVectorData>
::GetValidationCoordinatesList()
{
  return dynamic_cast<CoordinateList*>(this->itk::ProcessObject::GetOutput(7));
}



//The explicit instantiation part
template class ShpFileSampleGeneratorFast <otb::VectorImage<float, 2>, otb::VectorData<unsigned int,2> >;



