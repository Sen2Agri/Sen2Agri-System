/*
 * Copyright (C) 2005-2017 Centre National d'Etudes Spatiales (CNES)
 *
 * This file is part of Orfeo Toolbox
 *
 *     https://www.orfeo-toolbox.org/
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef otbOGRDataToClassStatisticsFilter_h
#define otbOGRDataToClassStatisticsFilter_h

#include "otbPersistentSamplingFilterBase.h"
#include "otbPersistentFilterStreamingDecorator.h"
#include "itkSimpleDataObjectDecorator.h"

namespace otb
{

/**
 * \class PersistentOGRDataToClassStatisticsFilter
 * 
 * \brief Persistent filter to compute class statistics based on vectors
 * 
 * \ingroup OTBSampling
 */
template<class TInputImage, class TMaskImage>
class ITK_EXPORT PersistentOGRDataToClassStatisticsFilter :
  public PersistentSamplingFilterBase<TInputImage, TMaskImage>
{
public:
    /** \class StatisticsMapAccumulator
     * \brief Holds statistics for each label of a label image
     *
     * Intended to store and update the following statistics:
     * -count
     * -sum of values
     * -sum of squared values
     * -min
     * -max
     *
     * TODO:
     * -Better architecture?
     * -Enrich with other statistics?
     * -Move this class in a dedicated source to enable its use by other otbStatistics stuff?
     *
     * \ingroup OTBStatistics
     */
    template<class TRealVectorPixelType>
    class StatisticsAccumulator
    {
    public:

        typedef typename TRealVectorPixelType::ValueType RealValueType;

        // Constructor (default)
        StatisticsAccumulator(){}

        // Constructor (initialize the accumulator with the given pixel)
        StatisticsAccumulator(const TRealVectorPixelType & pixel)
        {
            m_Pixels.push_back(pixel);

            m_Count = pixel;
            m_CountInvalid = pixel;
            m_Sum = pixel;
            m_Min = pixel;
            m_Max = pixel;
            m_SqSum = pixel;
            for (unsigned int band = 0 ; band < m_SqSum.GetSize() ; band++) {
                if (checkValid(pixel[band])) {
                    m_Count[band] = 1;
                    m_CountInvalid[band] = 0;
                    m_SqSum[band] *= m_SqSum[band];
                } else {
                    m_CountInvalid[band] = 1;
                    m_Count[band] = 0;
                    m_Sum[band] = 0;
                    m_Min[band] = 0;
                    m_Max[band] = 0;
                    m_SqSum[band] = 0;
                }
            }
        }

        // Constructor (other)
        StatisticsAccumulator(const StatisticsAccumulator & other)
        {
            m_Pixels = other.m_Pixels;

            m_Count = other.m_Count;
            m_CountInvalid = other.m_CountInvalid;
            m_Sum = other.m_Sum;
            m_Min = other.m_Min;
            m_Max = other.m_Max;
            m_SqSum = other.m_SqSum;
        }

        // Destructor
        ~StatisticsAccumulator(){}

        // Function update (pixel)
        void Update(const TRealVectorPixelType & pixel)
        {
            m_Pixels.push_back(pixel);

            const unsigned int nBands = pixel.GetSize();
            for (unsigned int band = 0 ; band < nBands ; band ++ )
            {
                const RealValueType value = pixel[band];
                if (checkValid(value))
                {
                    m_Count[band]++;
                    const RealValueType sqValue = value * value;
                    UpdateValues(value, sqValue, value, value,
                           m_Sum[band], m_SqSum[band], m_Min[band], m_Max[band]);
                } else {
                    m_CountInvalid[band]++;
                }
            }
        }

        // Function update (self)
        void Update(const StatisticsAccumulator & other)
        {
            m_Pixels.insert(m_Pixels.end(), other.m_Pixels.begin(), other.m_Pixels.end());

            m_Count += other.m_Count;
            m_CountInvalid += other.m_CountInvalid;
            const unsigned int nBands = other.m_Sum.GetSize();
            for (unsigned int band = 0 ; band < nBands ; band ++ )
            {
                UpdateValues(other.m_Sum[band], other.m_SqSum[band], other.m_Min[band], other.m_Max[band],
                       m_Sum[band], m_SqSum[band], m_Min[band], m_Max[band]);
            }
        }

        bool checkValid(const RealValueType &value)
        {
            if ((value != 0.0) && (value != -10000))
            {
                return true;
            }
            return false;
        }

        // Accessors
        itkGetMacro(Sum, TRealVectorPixelType);
        itkGetMacro(SqSum, TRealVectorPixelType);
        itkGetMacro(Min, TRealVectorPixelType);
        itkGetMacro(Max, TRealVectorPixelType);
        itkGetMacro(Count, TRealVectorPixelType);
        itkGetMacro(CountInvalid, TRealVectorPixelType);

    private:
        void UpdateValues(const RealValueType & otherSum, const RealValueType & otherSqSum,
                        const RealValueType & otherMin, const RealValueType & otherMax,
                        RealValueType & sum, RealValueType & sqSum,
                        RealValueType & min, RealValueType & max)
        {
            sum += otherSum;
            sqSum += otherSqSum;
            if (otherMin < min)
            {
                min = otherMin;
            }
            if (otherMax > max)
            {
                max = otherMax;
            }
        }

    public:
      std::vector<TRealVectorPixelType> m_Pixels;

    protected:
      TRealVectorPixelType m_Sum;
      TRealVectorPixelType m_SqSum;
      TRealVectorPixelType m_Min;
      TRealVectorPixelType m_Max;
      TRealVectorPixelType m_Count;
      TRealVectorPixelType m_CountInvalid;

    };

public:
  /** Standard Self typedef */
  typedef PersistentOGRDataToClassStatisticsFilter        Self;
  typedef PersistentSamplingFilterBase<
    TInputImage,
    TMaskImage>                                           Superclass;
  typedef itk::SmartPointer<Self>                         Pointer;
  typedef itk::SmartPointer<const Self>                   ConstPointer;

  typedef TInputImage                                     InputImageType;
  typedef typename InputImageType::Pointer                InputImagePointer;
  typedef typename InputImageType::RegionType             RegionType;
  typedef typename InputImageType::PointType              PointType;

  /** Wrap output type as DataObject */
  typedef std::map<std::string, unsigned long>      ClassCountMapType;
  typedef itk::SimpleDataObjectDecorator<ClassCountMapType>  ClassCountObjectType;
// NOT NEEDED FOR AGRICULTURAL PRACTICES
//  typedef std::map<unsigned long, unsigned long>    PolygonSizeMapType;
//  typedef itk::SimpleDataObjectDecorator<PolygonSizeMapType> PolygonSizeObjectType;

  typedef itk::DataObject::DataObjectPointerArraySizeType DataObjectPointerArraySizeType;

  typedef itk::VariableLengthVector<double>                             RealVectorPixelType;
  typedef StatisticsAccumulator<RealVectorPixelType>                    AccumulatorType;
  typedef std::map<std::string, AccumulatorType >                       AccumulatorMapType;
  typedef std::vector<AccumulatorMapType>                               AccumulatorMapCollectionType;

  typedef struct {
      RealVectorPixelType mean;
      RealVectorPixelType stdDev;
  } MeanStdDevValueType;

  typedef std::map<std::string, RealVectorPixelType >                   PixelValueMapType;
  typedef std::map<std::string, MeanStdDevValueType >                   PixeMeanStdDevlValueMapType;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Runtime information support. */
  itkTypeMacro(PersistentOGRDataToClassStatisticsFilter, PersistentSamplingFilterBase);

  void Synthetize(void) override;

  /** Reset method called before starting the streaming*/
  void Reset(void) override;

  /** the class count map is stored as output #2 */
  const ClassCountObjectType* GetClassCountOutput() const;
  ClassCountObjectType* GetClassCountOutput();

// NOT NEEDED FOR AGRICULTURAL PRACTICES
  /** the polygon size map is stored as output #3 */
//  const PolygonSizeObjectType* GetPolygonSizeOutput() const;
//  PolygonSizeObjectType* GetPolygonSizeOutput();

  // USED GetMeanStdDevValueMap INSTEAD

//  /** Return the computed Mean for each label in the input label image */
//   PixelValueMapType GetMeanValueMap() const;

//   /** Return the computed Standard Deviation for each label in the input label image */
//   PixelValueMapType GetStandardDeviationValueMap() const;

   /** Return the computed Mean and Standard Deviation for each label in the input label image */
   PixeMeanStdDevlValueMapType GetMeanStdDevValueMap() const;

   /** Return the computed Min for each label in the input label image */
   PixelValueMapType GetMinValueMap() const;

   /** Return the computed Max for each label in the input label image */
   PixelValueMapType GetMaxValueMap() const;

   /** Return the computed the number of valid pixels for each label in the input label image */
   PixelValueMapType GetValidPixelsCntMap() const;

   /** Return the computed the number of invalid pixels for each label in the input label image */
   PixelValueMapType GetInvalidPixelsCntMap() const;

  /** Make a DataObject of the correct type to be used as the specified
   * output. */
  itk::DataObject::Pointer MakeOutput(DataObjectPointerArraySizeType idx) override;
  using Superclass::MakeOutput;

  /** Set/Get macro for the flag specifying if the values should be converted to decibels */
  itkSetMacro(ConvertValuesToDecibels, bool);
  itkGetMacro(ConvertValuesToDecibels, bool);

  /** Set/Get macro for the flag specifying if min/max should be computed or not */
  itkSetMacro(ComputeMinMax, bool);
  itkGetMacro(ComputeMinMax, bool);

protected:
  /** Convert values to decibels before processing, default to 0 */
  bool m_ConvertValuesToDecibels;

protected:
  /** Constructor */
  PersistentOGRDataToClassStatisticsFilter();
  /** Destructor */
  ~PersistentOGRDataToClassStatisticsFilter() override {}

  /** Implement generic method called at each candidate position */
  void ProcessSample(const ogr::Feature& feature,
                     typename TInputImage::IndexType& imgIndex,
                     typename TInputImage::PointType& imgPoint,
                     const typename TInputImage::PixelType& value,
                     itk::ThreadIdType& threadid) override;

  /** Prepare temporary variables for the current feature */
  void PrepareFeature(const ogr::Feature& feature,
                      itk::ThreadIdType& threadid) override;

private:
  PersistentOGRDataToClassStatisticsFilter(const Self &) = delete;
  void operator =(const Self&) = delete;

  /** Number of pixels in all the polygons (per thread) */
  std::vector<unsigned long> m_NbPixelsThread;

  // NOT NEEDED FOR AGRICULTURAL PRACTICES
//  /** Number of pixels in each classes (per thread) */
//  std::vector<ClassCountMapType> m_ElmtsInClassThread;
//  /** Number of pixels in each polygons (per thread) */
//  std::vector<PolygonSizeMapType> m_PolygonThread;

  /** Class name of the current feature (per thread) */
  std::vector<std::string> m_CurrentClass;
  /** FID of the current feature (per thread) */
  std::vector<unsigned long> m_CurrentFID;

  AccumulatorMapCollectionType           m_AccumulatorMaps;

    // USED m_MeanStdDevRadiometricValue INSTEAD
//  PixelValueMapType                      m_MeanRadiometricValue;
//  PixelValueMapType                      m_StDevRadiometricValue;

  PixeMeanStdDevlValueMapType                    m_MeanStdDevRadiometricValue;

  PixelValueMapType                      m_MinRadiometricValue;
  PixelValueMapType                      m_MaxRadiometricValue;

  PixelValueMapType                      m_ValidPixelsCnt;
  PixelValueMapType                      m_InvalidPixelsCnt;

  bool m_ComputeMinMax;


};

/**
 * \class OGRDataToClassStatisticsFilter
 * 
 * \brief Computes class statistics based on vectors using a persistent filter
 * 
 * \sa PersistentOGRDataToClassStatisticsFilter
 *
 * \ingroup OTBSampling
 */
template<class TInputImage, class TMaskImage>
class ITK_EXPORT OGRDataToClassStatisticsFilter :
  public PersistentFilterStreamingDecorator<PersistentOGRDataToClassStatisticsFilter<TInputImage,TMaskImage> >
{
public:
  /** Standard Self typedef */
  typedef OGRDataToClassStatisticsFilter  Self;
  typedef PersistentFilterStreamingDecorator
    <PersistentOGRDataToClassStatisticsFilter
      <TInputImage,TMaskImage> >          Superclass;
  typedef itk::SmartPointer<Self>         Pointer;
  typedef itk::SmartPointer<const Self>   ConstPointer;

  typedef TInputImage                     InputImageType;
  typedef TMaskImage                      MaskImageType;
  typedef otb::ogr::DataSource            OGRDataType;
  
  typedef typename Superclass::FilterType             FilterType;
  typedef typename FilterType::ClassCountMapType      ClassCountMapType;
  typedef typename FilterType::ClassCountObjectType   ClassCountObjectType;
  // NOT NEEDED FOR AGRICULTURAL PRACTICES
//  typedef typename FilterType::PolygonSizeMapType     PolygonSizeMapType;
//  typedef typename FilterType::PolygonSizeObjectType  PolygonSizeObjectType;

  typedef typename FilterType::PixelValueMapType  PixelValueMapType;
  typedef typename FilterType::PixeMeanStdDevlValueMapType PixeMeanStdDevlValueMapType;


  /** Type macro */
  itkNewMacro(Self);

  /** Creation through object factory macro */
  itkTypeMacro(OGRDataToClassStatisticsFilter, PersistentFilterStreamingDecorator);

  using Superclass::SetInput;
  virtual void SetInput(const TInputImage* image);

  const TInputImage* GetInput();

  void SetOGRData(const otb::ogr::DataSource* data);
  const otb::ogr::DataSource* GetOGRData();

  void SetMask(const TMaskImage* mask);
  const TMaskImage* GetMask();

  void SetFieldName(const std::string &key);
  std::string GetFieldName();

  void SetConvertValuesToDecibels(bool exp);
  bool GetConvertValuesToDecibels();

  void SetComputeMinMax(bool exp);
  bool GetComputeMinMax();

  void SetLayerIndex(int index);
  int GetLayerIndex();

  void SetFieldValueFilterIds(const std::map<std::string, int> &filters);
  std::map<std::string, int> GetFieldValueFilterIds();


  const ClassCountObjectType* GetClassCountOutput() const;
  ClassCountObjectType* GetClassCountOutput();

// NOT NEEDED FOR AGRICULTURAL PRACTICES
//  const PolygonSizeObjectType* GetPolygonSizeOutput() const;
//  PolygonSizeObjectType* GetPolygonSizeOutput();

  // USED GetMeanStdDevValueMap INSTEAD
//  /** Return the computed Mean for each label in the input label image */
//   PixelValueMapType GetMeanValueMap() const;

//   /** Return the computed Standard Deviation for each label in the input label image */
//   PixelValueMapType GetStandardDeviationValueMap() const;

   /** Return the computed Mean and Standard Deviation for each label in the input label image */
   PixeMeanStdDevlValueMapType GetMeanStdDevValueMap() const;

   /** Return the computed Min for each label in the input label image */
   PixelValueMapType GetMinValueMap() const;

   /** Return the computed Max for each label in the input label image */
   PixelValueMapType GetMaxValueMap() const;

   /** Return the computed the number of valid pixels for each label in the input label image */
   PixelValueMapType GetValidPixelsCntMap() const;

   /** Return the computed the number of invalid pixels for each label in the input label image */
   PixelValueMapType GetInvalidPixelsCntMap() const;

protected:
  /** Constructor */
  OGRDataToClassStatisticsFilter() {}
  /** Destructor */
  ~OGRDataToClassStatisticsFilter() override {}

private:
  OGRDataToClassStatisticsFilter(const Self &) = delete;
  void operator =(const Self&) = delete;
};

} // end of namespace otb

#ifndef OTB_MANUAL_INSTANTIATION
#include "otbOGRDataToClassStatisticsFilter.txx"
#endif

#endif
