/**********************************************************************/
/*                    DO NOT MODIFY THIS HEADER                       */
/*             Swift, a Fourier spectral solver for MOOSE             */
/*                                                                    */
/*            Copyright 2024 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#include "LBMComputeVelocityMagnitude.h"
#include "LatticeBoltzmannProblem.h"
#include "LatticeBoltzmannMesh.h"

registerMooseObject("SwiftApp", LBMComputeVelocityMagnitude);

InputParameters
LBMComputeVelocityMagnitude::validParams()
{
  InputParameters params = LatticeBoltzmannOperator::validParams();
  params.addClassDescription("LBMComputeVelocityMagnitude object.");
  params.addRequiredParam<TensorInputBufferName>("velocity", "LBM velocity");
  return params;
}

LBMComputeVelocityMagnitude::LBMComputeVelocityMagnitude(const InputParameters & parameters)
  : LatticeBoltzmannOperator(parameters), _velocity(getInputBuffer("velocity"))
{
}

void
LBMComputeVelocityMagnitude::computeBuffer()
{
  const unsigned int & dim = _domain.getDim();
  switch (dim)
  {
    case 2:
      _u = torch::sqrt(_velocity.select(3, 0).pow(2) + _velocity.select(3, 1).pow(2));
      break;
    case 3:
      _u = torch::sqrt(_velocity.select(3, 0).pow(2) + _velocity.select(3, 1).pow(2) +
                       _velocity.select(3, 2).pow(2));
      break;
    default:
      mooseError("Unsupported dimension");
  }
  _lb_problem.maskedFillSolids(_u, 0);
}
