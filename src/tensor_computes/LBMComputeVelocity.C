/**********************************************************************/
/*                    DO NOT MODIFY THIS HEADER                       */
/*             Swift, a Fourier spectral solver for MOOSE             */
/*                                                                    */
/*            Copyright 2024 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#include "LBMComputeVelocity.h"
#include "LatticeBoltzmannProblem.h"
#include "LatticeBoltzmannStencilBase.h"

using namespace torch::indexing;

registerMooseObject("SwiftApp", LBMComputeVelocity);

InputParameters
LBMComputeVelocity::validParams()
{
  InputParameters params = LatticeBoltzmannOperator::validParams();
  params.addRequiredParam<TensorInputBufferName>("f", "Distribution function");
  params.addRequiredParam<TensorInputBufferName>("rho", "Density");
  params.addParam<TensorInputBufferName>("forces", "forces", "Force tensor");
  params.addParam<bool>("enable_forces", false, "Whether to enable forces or no");
  params.addParam<bool>("add_body_force", false, "Whether to enable forces or no");
  params.addParam<std::string>("body_force_x", "0.0", "Body force to be added in x-dir");
  params.addParam<std::string>("body_force_y", "0.0", "Body force to be added in y-dir");
  params.addParam<std::string>("body_force_z", "0.0", "Body force to be added in z-dir");
  params.addClassDescription("Compute object for macroscopic velocity reconstruction.");
  return params;
}

LBMComputeVelocity::LBMComputeVelocity(const InputParameters & parameters)
  : LatticeBoltzmannOperator(parameters),
    _f(getInputBuffer("f")),
    _rho(getInputBuffer("rho")),
    _force_tensor(getInputBuffer("forces")),
    _body_force_constant_x(_lb_problem.getConstant<Real>(getParam<std::string>("body_force_x"))),
    _body_force_constant_y(_lb_problem.getConstant<Real>(getParam<std::string>("body_force_y"))),
    _body_force_constant_z(_lb_problem.getConstant<Real>(getParam<std::string>("body_force_z")))
{
  if (getParam<bool>("add_body_force"))
  {
    std::vector<int64_t> shape = {_shape[0], _shape[1], _shape[2], _domain.getDim()};
    _body_forces = torch::zeros(shape, MooseTensor::floatTensorOptions());

    auto force_constants =
        torch::tensor({_body_force_constant_x, _body_force_constant_y, _body_force_constant_z},
                      MooseTensor::floatTensorOptions());

    for (int64_t d = 0; d < _domain.getDim(); d++)
    {
      auto t_index = torch::tensor({d}, MooseTensor::intTensorOptions());
      _body_forces.index_fill_(-1, t_index, force_constants[d]);
    }
  }
}

void
LBMComputeVelocity::computeBuffer()
{
  const unsigned int & dim = _domain.getDim();
  switch (dim)
  {
    case 3:
      _u.index_put_({Slice(), Slice(), Slice(), 0}, torch::sum(_f * _stencil._ex, 3) / _rho);
      _u.index_put_({Slice(), Slice(), Slice(), 1}, torch::sum(_f * _stencil._ey, 3) / _rho);
      _u.index_put_({Slice(), Slice(), Slice(), 2}, torch::sum(_f * _stencil._ez, 3) / _rho);
      break;
    case 2:
      _u.index_put_({Slice(), Slice(), Slice(), 0}, torch::sum(_f * _stencil._ex, 3) / _rho);
      _u.index_put_({Slice(), Slice(), Slice(), 1}, torch::sum(_f * _stencil._ey, 3) / _rho);
      break;
    default:
      mooseError("Unsupported dimension");
  }

  // include forces
  if (getParam<bool>("enable_forces"))
    _u = _u + _force_tensor / (2.0 * _rho.unsqueeze(3));

  if (getParam<bool>("add_body_force"))
    _u = _u + _body_forces / (2.0 * _rho.unsqueeze(3));

  _lb_problem.maskedFillSolids(_u, 0);
}
