/**********************************************************************/
/*                    DO NOT MODIFY THIS HEADER                       */
/*             Swift, a Fourier spectral solver for MOOSE             */
/*                                                                    */
/*            Copyright 2024 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#include "LBMComputeForces.h"
#include "LatticeBoltzmannProblem.h"

using namespace torch::indexing;

registerMooseObject("SwiftApp", LBMComputeForces);

InputParameters
LBMComputeForces::validParams()
{
  InputParameters params = LatticeBoltzmannOperator::validParams();
  // params.addRequiredParam<TensorInputBufferName>("f", "Distribution function");
  params.addParam<TensorInputBufferName>("temperature", "T", "Macroscopic temperature");
  params.addParam<TensorInputBufferName>("rho", "rho", "Macroscopic density");

  params.addParam<std::string>("rho0", "1.0", "Reference density");
  params.addParam<std::string>("T0", "1.0", "Reference temperature");
  params.addParam<std::string>("gravity", "0.001", "Gravitational accelaration");

  params.addParam<bool>("enable_gravity", false, "Whether to consider gravity");
  params.addParam<bool>("enable_buoyancy", false, "Whether to consider buoyancy");
  params.addParam<bool>(
      "enable_surface_forces", false, "Whether to consider surface tension in multiphase flow");

  params.addClassDescription("Compute object for LB forces");
  return params;
}

LBMComputeForces::LBMComputeForces(const InputParameters & parameters)
  : LatticeBoltzmannOperator(parameters),
    _reference_density((_lb_problem.getConstant<Real>(getParam<std::string>("rho0")))),
    _reference_temperature((_lb_problem.getConstant<Real>(getParam<std::string>("T0")))),
    _enable_gravity(getParam<bool>("enable_gravity")),
    _enable_buoyancy(getParam<bool>("enable_buoyancy")),
    _enable_surface_forces(getParam<bool>("enable_surface_forces")),
    _g(_lb_problem.getConstant<Real>(getParam<std::string>("gravity"))),
    _density_tensor(getInputBufferByName(getParam<TensorInputBufferName>("rho"))),
    _temperature(getInputBufferByName(getParam<TensorInputBufferName>("temperature")))
{
}

void
LBMComputeForces::computeGravity()
{
  _u = _u + _u.index_put_({Slice(), Slice(), Slice(), 1}, _g * _density_tensor);
}

void
LBMComputeForces::computeBuoyancy()
{
  // Boussinesq approximation of buoyancy
  _u = _u + _u.index_put_({Slice(), Slice(), Slice(), 1},
                          (_g * _reference_density) * (_temperature - _reference_temperature));
}

void
LBMComputeForces::computeSurfaceForces()
{
  // TBD
}

void
LBMComputeForces::computeBuffer()
{
  _u = torch::zeros_like(_u);

  if (_enable_gravity)
    computeGravity();
  if (_enable_buoyancy)
    computeBuoyancy();
  if (_enable_surface_forces)
    computeSurfaceForces();

  /// more forces can be added ?

  _lb_problem.maskedFillSolids(_u, 0);
}
