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
#ifndef __itkTimeVaryingVelocityFieldIntegrationImageFilter_hxx
#define __itkTimeVaryingVelocityFieldIntegrationImageFilter_hxx

#include "itkTimeVaryingVelocityFieldIntegrationImageFilter.h"

#include "itkImageRegionIteratorWithIndex.h"
#include "itkVectorLinearInterpolateImageFunction.h"

namespace itk
{

/*
 * TimeVaryingVelocityFieldIntegrationImageFilter class definitions
 */
template<class TTimeVaryingVelocityField, class TDisplacementField>
TimeVaryingVelocityFieldIntegrationImageFilter
  <TTimeVaryingVelocityField, TDisplacementField>
::TimeVaryingVelocityFieldIntegrationImageFilter()
{
  this->m_LowerTimeBound =  0.0,
  this->m_UpperTimeBound = 1.0,
  this->m_NumberOfIntegrationSteps = 100;

  this->SetNumberOfRequiredInputs( 1 );

  if( InputImageDimension - 1 != OutputImageDimension )
    {
    itkExceptionMacro( "The time-varying velocity field (input) should have "
      << "dimensionality of 1 greater than the deformation field (output). " );
    }

  typedef VectorLinearInterpolateImageFunction<TimeVaryingVelocityFieldType,
    RealType> DefaultVelocityFieldInterpolatorType;

  typename DefaultVelocityFieldInterpolatorType::Pointer
    velocityFieldInterpolator = DefaultVelocityFieldInterpolatorType::New();

  this->m_VelocityFieldInterpolator = velocityFieldInterpolator;

  typedef VectorLinearInterpolateImageFunction<DisplacementFieldType,
    RealType> DefaultDisplacementFieldInterpolatorType;

  typename DefaultDisplacementFieldInterpolatorType::Pointer
    deformationFieldInterpolator = DefaultDisplacementFieldInterpolatorType::New();

  this->m_DisplacementFieldInterpolator = deformationFieldInterpolator;
}

template<class TTimeVaryingVelocityField, class TDisplacementField>
TimeVaryingVelocityFieldIntegrationImageFilter
  <TTimeVaryingVelocityField, TDisplacementField>
::~TimeVaryingVelocityFieldIntegrationImageFilter()
{
}

template<class TTimeVaryingVelocityField, class TDisplacementField>
void
TimeVaryingVelocityFieldIntegrationImageFilter
  <TTimeVaryingVelocityField, TDisplacementField>
::GenerateOutputInformation()
{
  const TimeVaryingVelocityFieldType * input = this->GetInput();
  DisplacementFieldType * output = this->GetOutput();

  if( !input || !output )
    {
    return;
    }

  //
  // The ImageBase::CopyInformation() method ca not be used here
  // because these two images have different dimensions. Therefore
  // the individual elements must be copied for the common dimensions.
  //
  typedef typename DisplacementFieldType::SizeType      SizeType;
  typedef typename DisplacementFieldType::SpacingType   SpacingType;
  typedef typename DisplacementFieldType::PointType     OriginType;
  typedef typename DisplacementFieldType::DirectionType DirectionType;
  typedef typename DisplacementFieldType::RegionType    RegionType;

  SizeType size;
  SpacingType spacing;
  OriginType origin;
  DirectionType direction;

  typedef typename TimeVaryingVelocityFieldType::SizeType       InputSizeType;
  typedef typename TimeVaryingVelocityFieldType::SpacingType    InputSpacingType;
  typedef typename TimeVaryingVelocityFieldType::PointType      InputOriginType;
  typedef typename TimeVaryingVelocityFieldType::DirectionType  InputDirectionType;
  typedef typename TimeVaryingVelocityFieldType::RegionType     InputRegionType;

  const InputSpacingType & inputSpacing = input->GetSpacing();
  const InputOriginType & inputOrigin = input->GetOrigin();
  const InputDirectionType & inputDirection = input->GetDirection();
  const InputRegionType requestedRegion = input->GetRequestedRegion();
  const InputSizeType requestedSize = requestedRegion.GetSize();

  for( unsigned int i = 0; i < OutputImageDimension; i++ )
    {
    size[i] = requestedSize[i];
    spacing[i] = inputSpacing[i];
    origin[i] = inputOrigin[i];

    for( unsigned int j = 0; j < OutputImageDimension; j++ )
      {
      direction[i][j] = inputDirection[i][j];
      }
    }

  output->SetOrigin( origin );
  output->SetSpacing( spacing );
  output->SetDirection( direction );
  output->SetRegions( size );
}

template<class TTimeVaryingVelocityField, class TDisplacementField>
void
TimeVaryingVelocityFieldIntegrationImageFilter
  <TTimeVaryingVelocityField, TDisplacementField>
::BeforeThreadedGenerateData()
{
  VectorType zeroVector( 0.0 );

  this->AllocateOutputs();

  DisplacementFieldType * outputField =  this->GetOutput();

  outputField->FillBuffer( zeroVector );

  this->m_VelocityFieldInterpolator->SetInputImage( this->GetInput() );

  if( !this->m_InitialDiffeomorphism.IsNull() )
    {
    this->m_DisplacementFieldInterpolator->SetInputImage( this->m_InitialDiffeomorphism );
    }
}

template<class TTimeVaryingVelocityField, class TDisplacementField>
void
TimeVaryingVelocityFieldIntegrationImageFilter
  <TTimeVaryingVelocityField, TDisplacementField>
::ThreadedGenerateData( const OutputRegionType &region, ThreadIdType itkNotUsed( threadId ) )
{
  if( this->m_LowerTimeBound == this->m_UpperTimeBound )
    {
    return;
    }

  if( this->m_NumberOfIntegrationSteps == 0 )
    {
    return;
    }

  typename DisplacementFieldType::Pointer outputField = this->GetOutput();

  ImageRegionIteratorWithIndex<DisplacementFieldType> It( outputField, region );

  for( It.GoToBegin(); !It.IsAtEnd(); ++It )
    {
    PointType point;
    outputField->TransformIndexToPhysicalPoint( It.GetIndex(), point );
    VectorType displacement = this->IntegrateVelocityAtPoint( point );
    It.Set( displacement );
    }
}

template<class TTimeVaryingVelocityField, class TDisplacementField>
typename TimeVaryingVelocityFieldIntegrationImageFilter
  <TTimeVaryingVelocityField, TDisplacementField>::VectorType
TimeVaryingVelocityFieldIntegrationImageFilter
  <TTimeVaryingVelocityField, TDisplacementField>
::IntegrateVelocityAtPoint( const PointType & initialSpatialPoint )
{
  // Solve the initial value problem using fourth-order Runge-Kutta
  //    y' = f(t, y), y(t_0) = y_0

  VectorType zeroVector;
  zeroVector.Fill( 0.0 );

  // Initial conditions

  PointType startingSpatialPoint = initialSpatialPoint;
  if( !this->m_InitialDiffeomorphism.IsNull() )
    {
    if( this->m_DisplacementFieldInterpolator->IsInsideBuffer( startingSpatialPoint ) )
      {
      startingSpatialPoint += this->m_DisplacementFieldInterpolator->Evaluate( startingSpatialPoint );
      }
    }

  // Perform the integration

  // Need to know how to map the time dimension of the input image to the
  // assumed domain of [0,1].

  const TimeVaryingVelocityFieldType * inputField = this->GetInput();

  typename TimeVaryingVelocityFieldType::PointType spaceTimeOrigin = inputField->GetOrigin();

  typedef typename TimeVaryingVelocityFieldType::RegionType  RegionType;

  RegionType region = inputField->GetLargestPossibleRegion();

  typename RegionType::IndexType lastIndex = region.GetIndex();
  typename RegionType::SizeType size = region.GetSize();
  for( unsigned d = 0; d < InputImageDimension; d++ )
    {
    lastIndex[d] += ( size[d] - 1 );
    }
  typename TimeVaryingVelocityFieldType::PointType spaceTimeEnd;
  inputField->TransformIndexToPhysicalPoint( lastIndex, spaceTimeEnd );

  // Calculate the delta time used for integration
  const RealType deltaTime = vnl_math_abs( this->m_UpperTimeBound - this->m_LowerTimeBound ) /
    static_cast<RealType>( this->m_NumberOfIntegrationSteps - 1 );

  if( deltaTime == 0.0 )
    {
    return zeroVector;
    }

  const RealType timeOrigin = spaceTimeOrigin[InputImageDimension-1];
  const RealType timeEnd = spaceTimeEnd[InputImageDimension-1];
  const RealType timeSpan = timeEnd - timeOrigin;
  const RealType timeFraction = timeSpan * deltaTime;

  RealType timeSign = 1.0;
  if( this->m_UpperTimeBound < this->m_LowerTimeBound )
    {
    timeSign = -1.0;
    }

  RealType timePoint = timeOrigin + this->m_LowerTimeBound * timeSpan;

  VectorType displacement = zeroVector;

  for( unsigned int n = 0; n < this->m_NumberOfIntegrationSteps; n++ )
    {
    PointType spatialPoint = startingSpatialPoint + displacement;

    typename TimeVaryingVelocityFieldType::PointType x1;
    for( unsigned int d = 0; d < OutputImageDimension; d++ )
      {
      x1[d] = spatialPoint[d] + displacement[d];
      }
    x1[OutputImageDimension] = timePoint - timeSign * timeFraction;

    VectorType f1 = zeroVector;
    if( this->m_VelocityFieldInterpolator->IsInsideBuffer( x1 ) )
      {
      f1 = this->m_VelocityFieldInterpolator->Evaluate( x1 );
      }

    typename TimeVaryingVelocityFieldType::PointType x2;
    for( unsigned int d = 0; d < OutputImageDimension; d++ )
      {
      x2[d] = spatialPoint[d] + displacement[d] + f1[d] * 0.5 * deltaTime;
      }
    x2[OutputImageDimension] = timePoint - timeSign * timeFraction * 0.5;

    VectorType f2 = zeroVector;
    if( this->m_VelocityFieldInterpolator->IsInsideBuffer( x2 ) )
      {
      f2 = this->m_VelocityFieldInterpolator->Evaluate( x2 );
      }

    typename TimeVaryingVelocityFieldType::PointType x3;
    for( unsigned int d = 0; d < OutputImageDimension; d++ )
      {
      x3[d] = spatialPoint[d] + displacement[d] + f2[d] * 0.5 * deltaTime;
      }
    x3[OutputImageDimension] = timePoint - timeSign * timeFraction * 0.5;

    VectorType f3 = zeroVector;
    if( this->m_VelocityFieldInterpolator->IsInsideBuffer( x3 ) )
      {
      f3 = this->m_VelocityFieldInterpolator->Evaluate( x3 );
      }

    typename TimeVaryingVelocityFieldType::PointType x4;
    for( unsigned int d = 0; d < OutputImageDimension; d++ )
      {
      x4[d] = spatialPoint[d] + displacement[d] + f3[d] * deltaTime;
      }
    x4[OutputImageDimension] = timePoint;

    VectorType f4 = zeroVector;
    if( this->m_VelocityFieldInterpolator->IsInsideBuffer( x4 ) )
      {
      f4 = this->m_VelocityFieldInterpolator->Evaluate( x4 );
      }

    displacement += ( timeSign * deltaTime * ( f1 + f2 * 2.0 + f3 * 2.0 + f4 ) / 6.0 );
    timePoint += ( timeSign * timeFraction );
    }
  return displacement;
}

template<class TTimeVaryingVelocityField, class TDisplacementField>
void
TimeVaryingVelocityFieldIntegrationImageFilter
  <TTimeVaryingVelocityField, TDisplacementField>
::PrintSelf( std::ostream& os, Indent indent ) const
{
  Superclass::PrintSelf( os, indent );

  os << indent << "VelocityFieldInterpolator: "
    << this->m_VelocityFieldInterpolator << std::endl;
  os << indent << "LowerTimeBound: " << this->m_LowerTimeBound << std::endl;
  os << indent << "UpperTimeBound: " << this->m_UpperTimeBound << std::endl;
  os << indent << "NumberOfIntegrationSteps: "
    << this->m_NumberOfIntegrationSteps << std::endl;

  if( !this->m_InitialDiffeomorphism.IsNull() )
    {
    os << indent << "InitialDiffeomorphism: " << this->m_InitialDiffeomorphism << std::endl;
    os << indent << "DisplacementFieldInterpolator: "
      << this->m_DisplacementFieldInterpolator << std::endl;
    }
}

}  //end namespace itk

#endif
