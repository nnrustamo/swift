/**********************************************************************/
/*                    DO NOT MODIFY THIS HEADER                       */
/*             Swift, a Fourier spectral solver for MOOSE             */
/*                                                                    */
/*            Copyright 2024 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#include "LBMTensorUnitConverter.h"

registerMooseObject("SwiftApp", LBMTensorUnitConverter);

InputParameters
LBMTensorUnitConverter::validParams()
{
  InputParameters params = LatticeBoltzmannOperator::validParams();
  params.addRequiredParam<TensorInputBufferName>("tensor_buffer",
                                                 "The macroscopic buffer variable to convert");
  params.addRequiredParam<std::vector<std::string>>("constant",
                                                    "The scalar conversion constant names");
  params.addClassDescription("LBMConstantTensor object.");
  return params;
}

LBMTensorUnitConverter::LBMTensorUnitConverter(const InputParameters & parameters)
  : LatticeBoltzmannOperator(parameters),
    _tensor_buffer(getInputBuffer(getParam<TensorInputBufferName>("tensor_buffer"))),
    _conversion_constant(_lb_problem.getConstant<Real>(getParam<TensorInputBufferName>("constant")))
{
}

void
LBMTensorUnitConverter::computeBuffer()
{
  _u = _tensor_buffer * _conversion_constant;
}
