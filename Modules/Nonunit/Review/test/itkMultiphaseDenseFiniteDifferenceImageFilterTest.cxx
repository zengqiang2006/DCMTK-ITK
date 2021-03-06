/*=========================================================================
 *
 *  Copyright Insight Software Consortium
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include "itkMultiphaseDenseFiniteDifferenceImageFilter.h"
#include "itkScalarChanAndVeseLevelSetFunction.h"

namespace itk
{

template < class TInputImage, class TFeatureImage, class TOutputImage,
  class TFiniteDifferenceFunction, typename TIdCell >
class MultiphaseDenseFiniteDifferenceImageFilterTestHelper
  : public MultiphaseDenseFiniteDifferenceImageFilter<
      TInputImage, TFeatureImage, TOutputImage,
      TFiniteDifferenceFunction, TIdCell >
{
public:
  /** Standard class typedefs. */
  typedef MultiphaseDenseFiniteDifferenceImageFilterTestHelper      Self;
  typedef MultiphaseDenseFiniteDifferenceImageFilter< TInputImage, TFeatureImage,
    TOutputImage,TFiniteDifferenceFunction >                        Superclass;
  typedef SmartPointer<Self>                                        Pointer;
  typedef SmartPointer<const Self>                                  ConstPointer;

  /** Run-time type information (and related methods) */
  itkTypeMacro( MultiphaseDenseFiniteDifferenceImageFilterTestHelper, MultiphaseDenseFiniteDifferenceImageFilter );

  itkNewMacro( Self );

  virtual void AllocateUpdateBuffer() {}

  typedef typename Superclass::TimeStepType   TimeStepType;

  virtual void ApplyUpdate(TimeStepType itkNotUsed(dt) ) {}

  virtual TimeStepType CalculateChange()
    {
    return TimeStepType( 1.0 );
    }

  virtual void CopyInputToOutput() {}

};

}

int itkMultiphaseDenseFiniteDifferenceImageFilterTest( int, char* [] )
{
  const unsigned int Dimension = 3;

  typedef itk::Image< double, Dimension >         LevelSetImageType;
  typedef itk::Image< float, Dimension >          FeatureImageType;
  typedef itk::Image< unsigned char, Dimension >  OutputImageType;

  typedef itk::ScalarChanAndVeseLevelSetFunctionData< LevelSetImageType, FeatureImageType >
    DataHelperType;
  typedef itk::ConstrainedRegionBasedLevelSetFunctionSharedData< LevelSetImageType,
    FeatureImageType, DataHelperType >            SharedDataHelperType;

  typedef itk::ScalarChanAndVeseLevelSetFunction< LevelSetImageType, FeatureImageType,
    SharedDataHelperType >                        RegionBasedLevelSetFunctionType;

  RegionBasedLevelSetFunctionType::Pointer function = RegionBasedLevelSetFunctionType::New();
  if( function.IsNull() )
    {
    return EXIT_FAILURE;
    }

  typedef unsigned long IdCellType;

  typedef itk::MultiphaseDenseFiniteDifferenceImageFilterTestHelper< LevelSetImageType,
    FeatureImageType, OutputImageType, RegionBasedLevelSetFunctionType,
    IdCellType >  FilterType;

  FilterType::Pointer filter = FilterType::New();

  std::cout << "GetNameOfClass() = " << filter->GetNameOfClass() << std::endl;
  filter->Print( std::cout );

  return EXIT_SUCCESS;
}
