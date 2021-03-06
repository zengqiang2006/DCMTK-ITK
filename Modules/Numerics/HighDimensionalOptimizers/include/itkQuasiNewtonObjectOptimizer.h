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
#ifndef __itkQuasiNewtonObjectOptimizer_h
#define __itkQuasiNewtonObjectOptimizer_h

#include "itkArray2D.h"
#include "itkGradientDescentObjectOptimizer.h"

#include "vnl/algo/vnl_matrix_inverse.h"
#include "vnl/algo/vnl_determinant.h"

namespace itk
{
/** \class QuasiNewtonObjectOptimizer
 * \brief Implement a Quasi-Newton optimizer with BFGS Hessian estimation.
 *
 * Second order approximation of the cost function is usually more efficient
 * since it estimates the descent or ascent direction more precisely. However,
 * computation of Hessian is usually expensive or unavailable. Alternatively
 * Quasi-Newton methods can estimate a Hessian from the gradients in previous
 * steps. Here a specific Quasi-Newton method, BFGS, is used to compute the
 * Quasi-Newton steps.
 *
 * The Quasi-Newton method doesn't produce a valid step sometimes, ex. when
 * the metric function is not a convex locally. In this scenario, the gradient
 * step is used after being scaled properly.
 *
 * A helper member object, m_ScalesEstimator may be set to estimate parameter
 * scales and step scales. A step scale measures the magnitude of a step and
 * is used for learning rate computation.

 * When m_ScalesEstimator is set, SetMaximumNewtonStepSizeInPhysicalUnits()
 * may be called to set the maximum step size. If it is not called,
 * m_MaximumNewtonStepSizeInPhysicalUnits defaults to lamda *
 * OptimizerParameterScalesEstimator::GetMaximumStepSize(), where lamda is
 * in [1,5].
 *
 * When m_ScalesEstimator is not set, the parameter scales and learning rates
 * defaults to ones, or can be set by users manually.
 *
 * \ingroup ITKHighDimensionalOptimizers
 */
class ITK_EXPORT QuasiNewtonObjectOptimizer:
  public GradientDescentObjectOptimizer
{
public:
  /** Standard class typedefs. */
  typedef QuasiNewtonObjectOptimizer        Self;
  typedef GradientDescentObjectOptimizer    Superclass;
  typedef SmartPointer< Self >              Pointer;
  typedef SmartPointer< const Self >        ConstPointer;

  /** Method for creation through the object factory. */
  itkNewMacro(Self);

  /** Run-time type information (and related methods). */
  itkTypeMacro(QuasiNewtonObjectOptimizer, GradientDescentObjectOptimizer);

  typedef Superclass::InternalComputationValueType    InternalComputationValueType;

  /** Type for Hessian matrix in the Quasi-Newton method */
  typedef itk::Array2D<InternalComputationValueType>  HessianType;

  /** Type for an array of Hessian matrix for local support */
  typedef std::vector<HessianType>                    HessianArrayType;

  /** Threader for Quasi-Newton method */
  typedef Array1DToData<Self>                         QuasiNewtonThreaderType;

  /** Start and run the optimization */
  virtual void StartOptimization();

  /** Set the maximum tolerable number of iteration without any progress */
  itkSetMacro(MaximumIterationsWithoutProgress, SizeValueType);

  /** Set the maximum step size. */
  itkSetMacro(MaximumNewtonStepSizeInPhysicalUnits, InternalComputationValueType);

  /** Get the most recent Newton step. */
  itkGetConstReferenceMacro( NewtonStep, DerivativeType );

protected:

  /** The maximum tolerable number of iteration without any progress */
  SizeValueType   m_MaximumIterationsWithoutProgress;

  /** The information about the current step */
  ParametersType  m_CurrentPosition;
  ParametersType  m_OptimalStep;

  /** The information about the previous step */
  MeasureType     m_PreviousValue;
  ParametersType  m_PreviousPosition;
  DerivativeType  m_PreviousGradient;

  /** The best value so far and relevant information */
  MeasureType     m_BestValue;
  ParametersType  m_BestPosition;
  SizeValueType   m_BestIteration;

  /** The Quasi-Newton step */
  DerivativeType  m_NewtonStep;

  /** Warning message during Quasi-Newton step estimation */
  std::string     m_NewtonStepWarning;

  /** The maximum Quasi-Newton step size to restrict learning rates. */
  InternalComputationValueType  m_MaximumNewtonStepSizeInPhysicalUnits;

  /** The Hessian with local support */
  HessianArrayType  m_HessianArray;

  /** Valid flag for the Quasi-Newton steps */
  std::vector<bool> m_NewtonStepValidFlags;

  /** Estimate a Newton step */
  virtual void EstimateNewtonStep();

  /**
   * Estimate the quasi-newton step over a given index range.
   */
  virtual void EstimateNewtonStepOverSubRange( const IndexRangeType& subrange );

  /** Estimate the next Hessian and step with BFGS method.
   *  The details of the method are described at
   *  http://en.wikipedia.org/wiki/BFGS_method .
   */
  virtual bool ComputeHessianAndStepWithBFGS(IndexValueType location);

  /** Reset the Hessian to identity matrix and the Newton step to zeros. */
  virtual void ResetNewtonStep(IndexValueType location);

  /**
   * Combine a gradient step with a Newton step. The Newton step will be used
   * when it is valid. Otherwise the gradient step will be used.
   */
  void CombineGradientNewtonStep(void);

  /**
   *  Estimate and apply the learning rate(s) for a combined Newton step.
   *  A combined Newton step uses the Newton step by default and the gradient
   *  step when the Newton step is not valid.
   *
   *  The learning rate is less than 1.0 and is restricted by
   *  m_MaximumNewtonStepSizeInPhysicalUnits.
   */
  void ModifyCombinedNewtonStep();

  /** Advance one step using the Quasi-Newton step or the gradient step. */
  virtual void AdvanceOneStep(void);

  QuasiNewtonObjectOptimizer();
  virtual ~QuasiNewtonObjectOptimizer();

  virtual void PrintSelf(std::ostream & os, Indent indent) const;

private:
  QuasiNewtonObjectOptimizer(const Self &);     //purposely not implemented
  void operator=(const Self &);           //purposely not implemented

  /**
   * Estimate the quasi-newton step in a thread.
   */
  static void EstimateNewtonStepThreaded( const IndexRangeType& rangeForThread,
                          ThreadIdType,
                          Self *holder );

  /** Threader for Newton step estimation. */
  QuasiNewtonThreaderType::Pointer      m_EstimateNewtonStepThreader;
};
} // end namespace itk

#endif
