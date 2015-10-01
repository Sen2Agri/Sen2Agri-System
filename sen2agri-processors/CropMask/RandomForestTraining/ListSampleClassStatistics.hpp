/*
 * ListSampleClassStatistics.hpp
 *
 *  Created on: 1/9/2014
 *      Author: silvia
 */

#ifndef LISTSAMPLECLASSSTATISTICS_HPP_
#define LISTSAMPLECLASSSTATISTICS_HPP_


#include "itkListSample.h"
#include "itkVariableSizeMatrix.h"
#include "otbVectorImage.h"
#include "otbVectorData.h"

#include "otbImage.h"
#include <string>
#include <fstream>
#include <iomanip>


class ListSampleClassStatistics
{
	public:
		typedef int LabelPixelType;
		typedef itk::FixedArray<LabelPixelType, 1> LabelType;
		typedef itk::Statistics::ListSample<LabelType> ListLabelType;

        typedef short PixelType;
        typedef otb::VectorImage<PixelType, 2>::PixelType SampleType;
		typedef itk::Statistics::ListSample<SampleType> ListSampleType;


		ListSampleClassStatistics(ListLabelType::Pointer Labels , ListSampleType::Pointer Values, unsigned int NbFeatures)
		{
			typename ListLabelType::Iterator iter = Labels->Begin();
			typename ListSampleType::Iterator iterValues = Values->Begin();

			_NbFeatures = NbFeatures;

			_ClassLabels.insert( std::pair<LabelPixelType, std::string>(1,std::string("CROP")));
			_ClassLabels.insert( std::pair<LabelPixelType, std::string>(0,std::string("NO_CROP")));

			_ClassMean.clear();
			_ClassVariance.clear();

			std::vector<float> meanCrop(_NbFeatures);
			std::vector<float> varianceCrop(_NbFeatures);
			std::vector<float> meanNoCrop(_NbFeatures);
			std::vector<float> varianceNoCrop(_NbFeatures);
			_ClassMean.insert ( std::pair<LabelPixelType, std::vector<float>>(static_cast<LabelPixelType>(0),meanCrop) );
			_ClassMean.insert ( std::pair<LabelPixelType, std::vector<float>>(static_cast<LabelPixelType>(1),meanNoCrop) );

			_ClassVariance.insert ( std::pair<LabelPixelType, std::vector<float>>(static_cast<LabelPixelType>(0),varianceCrop) );
			_ClassVariance.insert ( std::pair<LabelPixelType, std::vector<float>>(static_cast<LabelPixelType>(1),varianceNoCrop) );

			while( iter != Labels->End() )
			{

				SampleType Sample = iterValues.GetMeasurementVector();
				LabelType SampleLabel = iter.GetMeasurementVector();

				_ClassSize[SampleLabel[0]]++;
				std::map<LabelPixelType, std::vector<float>>::iterator itmap = _ClassMean.find(SampleLabel[0]);

				for (auto Id = 0; Id < itmap->second.size(); ++Id )
				{
				   itmap->second.at(Id) = itmap->second.at(Id) + Sample[Id] ;
				}
				_ClassMean.insert ( std::pair<LabelPixelType, std::vector<float>>(SampleLabel[0],itmap->second) );


			 	++ iter;
			 	++ iterValues;

			 }


			 for (auto itmap =  _ClassMean.begin(); itmap !=  _ClassMean.end(); ++itmap)
			 {

				for (auto Id = 0; Id <(itmap->second).size(); ++Id )
				{
					itmap->second.at(Id) =  static_cast<float>(itmap->second.at(Id) / _ClassSize[itmap->first]) ;
				}
				_ClassMean.insert( std::pair<LabelPixelType, std::vector<float>>(itmap->first,itmap->second) );

			 }


			 //Variance

			 iter = Labels->Begin();
			 iterValues = Values->Begin();

			 while( iter != Labels->End() )
			 {

			 	SampleType Sample = iterValues.GetMeasurementVector();
			 	LabelType SampleLabel = iter.GetMeasurementVector();

			 	std::map<LabelPixelType, std::vector<float>>::iterator itmapV = _ClassVariance.find(SampleLabel[0]);
			 	std::vector<float> mean = _ClassMean[SampleLabel[0]];

			 	for (auto Id = 0; Id < (itmapV->second).size(); ++Id )
			 	{

			 		float aux = (Sample[Id]*Sample[Id]) - (mean[Id]*mean[Id]);
			 		(itmapV->second).at(Id) = (itmapV->second).at(Id) + aux;

			 	}
			 	_ClassVariance.insert ( std::pair<LabelPixelType, std::vector<float>>(SampleLabel[0],itmapV->second) );


			 	++ iter;
			 	++ iterValues;
			}


			 for (auto itmapV =  _ClassVariance.begin(); itmapV !=  _ClassVariance.end(); ++itmapV)
			 {

				for (auto Id = 0; Id <itmapV->second.size(); ++Id )
				{
					itmapV->second.at(Id) =  static_cast<float>(itmapV->second.at(Id) / _ClassSize[itmapV->first]) ;
				}
				_ClassVariance.insert( std::pair<LabelPixelType, std::vector<float>>(itmapV->first,itmapV->second) );

			  }
		}


		void writeResultsMean( std::string filename)
		{
			std::ofstream myfile;
			myfile.open (filename);
			myfile.precision(3);

			 std::map<LabelPixelType, std::vector<float>>::const_iterator itmap;

			 LabelPixelType labelID = 0;
			for (itmap = _ClassMean.begin(); itmap != _ClassMean.end(); ++itmap)
			{
				myfile << std::setw(15) << _ClassLabels[itmap->first] << " ";
				for (auto Id = 0; Id < (itmap->second).size(); ++Id )
				myfile << std::setw(15) << (itmap->second).at(Id);
				myfile<< std::endl;

			 }

			myfile.close();
		}


		void writeResultsVariance( std::string filename)
		{
			std::ofstream myfile;
			myfile.open (filename);
			myfile.precision(3);

			 std::map<LabelPixelType, std::vector<float>>::const_iterator itmap;

			 LabelPixelType labelID = 0;
			for (itmap = _ClassVariance.begin(); itmap != _ClassVariance.end(); ++itmap)
			{
				myfile << std::setw(15) << _ClassLabels[itmap->first] << " ";
				for (auto Id = 0; Id < (itmap->second).size(); ++Id )
				myfile << std::setw(15) << std::sqrt((itmap->second).at(Id));
				myfile<< std::endl;

			 }

			myfile.close();
		}

	private:

		unsigned int _NbFeatures = 0;
		std::map<LabelPixelType, std::string> _ClassLabels;

		std::map<LabelPixelType, std::vector<float>> _ClassMean;
		std::map<LabelPixelType, std::vector<float>> _ClassVariance;
		std::map<LabelPixelType, unsigned int> _ClassSize;

};





#endif /* LISTSAMPLECLASSSTATISTICS_HPP_ */
