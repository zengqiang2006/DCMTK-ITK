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

#ifndef __itkLevelSetEquationChanAndVeseExternalTerm_hxx
#define __itkLevelSetEquationChanAndVeseExternalTerm_hxx

#include "itkLevelSetEquationChanAndVeseExternalTerm.h"

namespace itk
{
template< class TInput, class TLevelSetContainer >
LevelSetEquationChanAndVeseExternalTerm< TInput, TLevelSetContainer >
::LevelSetEquationChanAndVeseExternalTerm()
{
  this->m_TermName = "External Chan And Vese term";
  this->m_RequiredData.insert( "Value" );
}

template< class TInput, class TLevelSetContainer >
LevelSetEquationChanAndVeseExternalTerm< TInput, TLevelSetContainer >
::~LevelSetEquationChanAndVeseExternalTerm()
{
}


template< class TInput, class TLevelSetContainer >
void LevelSetEquationChanAndVeseExternalTerm< TInput, TLevelSetContainer >
::ComputeProduct( const LevelSetInputIndexType& iP, LevelSetOutputRealType& prod )
{
  this->ComputeProductTerm( iP, prod );
  LevelSetPointer levelSet = this->m_LevelSetContainer->GetLevelSet( this->m_CurrentLevelSetId );
  LevelSetOutputRealType value = levelSet->Evaluate( iP );
  prod *= -(1 - this->m_Heaviside->Evaluate( -value ) );
}

template< class TInput, class TLevelSetContainer >
void LevelSetEquationChanAndVeseExternalTerm< TInput, TLevelSetContainer >
::ComputeProductTerm( const LevelSetInputIndexType& iP, LevelSetOutputRealType& prod )
{
  prod = -1 * NumericTraits< LevelSetOutputRealType >::One;

  DomainMapImageFilterType * domainMapFilter = this->m_LevelSetContainer->GetDomainMapFilter();

  bool isDomainMapProvided = false;

  if( domainMapFilter != NULL )
    {
    if( domainMapFilter->GetDomainMap().size() > 0 )
      {
      isDomainMapProvided = true;
      }
    }

  if( isDomainMapProvided )
    {
    CacheImageType * cacheImage = domainMapFilter->GetOutput();
    const LevelSetIdentifierType id = cacheImage->GetPixel( iP );

    typedef typename DomainMapImageFilterType::DomainMapType DomainMapType;
    const DomainMapType domainMap = domainMapFilter->GetDomainMap();
    typename DomainMapType::const_iterator levelSetMapItr = domainMap.find(id);

    if( levelSetMapItr != domainMap.end() )
      {
      const IdListType lout = levelSetMapItr->second.m_List;

      LevelSetIdentifierType kk;
      LevelSetPointer levelSet;
      LevelSetOutputRealType value;

      for( IdListConstIterator lIt = lout.begin(); lIt != lout.end(); ++lIt )
        {
        kk = *lIt - 1;
        if( kk != this->m_CurrentLevelSetId )
          {
          levelSet = this->m_LevelSetContainer->GetLevelSet( kk );
          value = levelSet->Evaluate( iP );
          prod *= ( NumericTraits< LevelSetOutputRealType >::One - this->m_Heaviside->Evaluate( -value ) );
          }
        }
      }
    }
  else
    {
    LevelSetIdentifierType kk;
    LevelSetPointer levelSet;
    LevelSetOutputRealType value;

    typename LevelSetContainerType::Iterator lsIt = this->m_LevelSetContainer->Begin();

    while( lsIt != this->m_LevelSetContainer->End() )
      {
      kk = lsIt->GetIdentifier();
      if( kk != this->m_CurrentLevelSetId )
        {
        levelSet = this->m_LevelSetContainer->GetLevelSet( kk );
        value = levelSet->Evaluate( iP );
        prod *= ( NumericTraits< LevelSetOutputRealType >::One - this->m_Heaviside->Evaluate( -value ) );
        }
      ++lsIt;
      }
    }
}

template< class TInput, class TLevelSetContainer >
void LevelSetEquationChanAndVeseExternalTerm< TInput, TLevelSetContainer >
::UpdatePixel( const LevelSetInputIndexType& iP, const LevelSetOutputRealType & oldValue,
             const LevelSetOutputRealType & newValue )
{
  // Compute the product factor
  LevelSetOutputRealType prod;

  this->ComputeProductTerm( iP, prod );

  // For each affected h val: h val = new hval (this will dirty some cvals)
  InputPixelType input = this->m_Input->GetPixel( iP );

  const LevelSetOutputRealType oldH = this->m_Heaviside->Evaluate( -oldValue );
  const LevelSetOutputRealType newH = this->m_Heaviside->Evaluate( -newValue );
  const LevelSetOutputRealType change = oldH - newH;//(1 - newH) - (1 - oldH);

  // Determine the change in the product factor
  const LevelSetOutputRealType productChange = -( prod * change );

  this->m_TotalH += change;
  this->m_TotalValue += input * productChange;
}

}

#endif
