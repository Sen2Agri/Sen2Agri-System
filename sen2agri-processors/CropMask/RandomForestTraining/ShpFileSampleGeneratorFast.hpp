/*
 * ShpFileSampleGeneratorFast.hpp
 *
 *  Created on: 19/1/2015
 *      Author: silvia
 */

#ifndef SHPFILESAMPLEGENERATORFast_HPP_
#define SHPFILESAMPLEGENERATORFast_HPP_


#include "itkProcessObject.h"
#include "itkListSample.h"
#include "itkPreOrderTreeIterator.h"
#include "itkImageRegionConstIteratorWithIndex.h"
//#include "itkMersenneTwisterRandomVariateGenerator.h"


template <class TImage, class TVectorData>
class ITK_EXPORT ShpFileSampleGeneratorFast :public itk::ProcessObject
{

public:


	typedef ShpFileSampleGeneratorFast Self;
	typedef itk::ProcessObject Superclass;
	typedef itk::SmartPointer<Self> Pointer;
	typedef itk::SmartPointer<const Self> ConstPointer;



	itkNewMacro(Self);
	itkTypeMacro(ShpFileSampleGeneratorFast, itk::ProcessObject);

	typedef int ClassLabelType;

	typedef typename TImage::PixelType SampleType;
	typedef itk::Statistics::ListSample<SampleType> ListSampleType;


	 typedef itk::FixedArray<ClassLabelType, 1> LabelType; //note could be templated by an std:::string
	 typedef itk::Statistics::ListSample<LabelType> ListLabelType;

	typedef itk::FixedArray<unsigned int, 2> CoordinateType; //note could be templated by an std:::string
	typedef itk::Statistics::ListSample<CoordinateType> CoordinateList;


	// Build the outputs
	typedef itk::DataObject::Pointer DataObjectPointer;
	virtual DataObjectPointer MakeOutput(unsigned int idx);

	ShpFileSampleGeneratorFast( );

	void SetInputImage(TImage * InputImage);

	const TImage* GetInput() const;

	void SetInputVectorData(const TVectorData * VectorDataInput);

	const TVectorData* GetInputVectorData() const;

	void GenerateClassStatistics();

	void GenerateSampleLists();

	int GetNumberOfClasses( );

	std::map<int, std::string>*  GetInfoLabelsOfClasses ( );

	std::map< int, unsigned int>*  GetInfoSamplesOfClasses ( );

	void PrintInformation( );

	void SetSeed(unsigned int seed);

	void SetLimitationofMaximumTrainingSamples( unsigned int LimitationofMaximumTrainingSamples);

	/** Returns the Training ListSample as a data object */
	ListSampleType * GetTrainingListSample();

	/** Returns the Trainingn label ListSample as a data object */
	ListLabelType * GetTrainingListLabel();

	/** Returns the Trainingn label ListSample as a data object */
	ListLabelType * GetTrainingListLabelCrop();

	/** Returns the label sample list as a data object */
	ListSampleType * GetValidationListSample();

	/** Returns the label sample list as a data object */
	ListLabelType * GetValidationListLabel();

	 /** Returns the label sample list as a data object */
	 ListLabelType * GetValidationListLabelCrop();

	/** Returns the image coordinates sample list used in the training as a data object */
  	CoordinateList* GetTrainingCoordinatesList();
 	
	/** Returns the image coordinates sample list used in the validation as a data object */
	CoordinateList* GetValidationCoordinatesList();



private:

	typedef typename TVectorData::DataNodeType DataNodeType;
	typedef typename DataNodeType::PolygonType PolygonType;
	typedef typename DataNodeType::PolygonPointerType PolygonPointerType;
	typedef typename DataNodeType::PolygonListType PolygonListType;
	typedef typename DataNodeType::PolygonListPointerType PolygonListPointerType;
	typedef itk::PreOrderTreeIterator<typename TVectorData::DataTreeType> TreeIteratorType;

	double GetPolygonAreaPixels(DataNodeType* polygonDataNode, TImage* image);

	int _NumberOfClasses;

	unsigned int  _NumberOfCropSamples;

	unsigned int _NumberOfNoCropSamples;

	unsigned int _MaximumTrainingSamples;

	unsigned int _seed;

	unsigned int _LimitationofMaximumTrainingSamples;

    std::string _ClassKey;

	std::string _CropClassKey;

	std::string _LabelClassKey;

	std::map<ClassLabelType, std::string> _ClassesLabels;

	std::map<ClassLabelType, unsigned int> _ClassesSize;

	std::map<ClassLabelType, bool> _ClassesCrop;

	typename ListSampleType::Pointer _Samples;

	typename CoordinateList::Pointer _CoordinatesList;

	std::map<ClassLabelType,  std::vector<unsigned int>> _CoordinateSamples;



};

#ifndef ITK_MANUAL_INSTANTIATION
#include "ShpFileSampleGeneratorFast.txx"
#endif

#endif /* SHPFILESAMPLEGENERATORFast_HPP_ */
