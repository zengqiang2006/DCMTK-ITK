set(DOCUMENTATION "This module contains ITK classes than encapsulate numerical
optimizers using a new hierarchy developed for the needs of registration with high-dimensional transforms. These optimizers will NOT work with the metrics in Registration/Common, but rather with the new metrics in Registration/HighDimensionalMetrics.")

itk_module(ITKHighDimensionalOptimizers
  DEPENDS
    ITKCommon
    ITKOptimizers
    ITKTransform
    ITKImageGrid
  TEST_DEPENDS
    ITKTestKernel
    ITKHighDimensionalMetrics
  DESCRIPTION
    "${DOCUMENTATION}"
)

# ITKOptimizers dependency added to get itkCostFunction for itkSingleValuedHighDimensionalCostFunction
