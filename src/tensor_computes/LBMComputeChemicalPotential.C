/**********************************************************************/
/*                    DO NOT MODIFY THIS HEADER                       */
/*             Swift, a Fourier spectral solver for MOOSE             */
/*                                                                    */
/*            Copyright 2024 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#include "LBMComputeChemicalPotential.h"

registerMooseObject("SwiftApp", LBMComputeChemicalPotential);

InputParameters
LBMComputeChemicalPotential::validParams()
{
  InputParameters params = LatticeBoltzmannOperator::validParams();
  params.addClassDescription("Compute LB checmial potential for pahse field coupling.");
  params.addRequiredParam<TensorInputBufferName>("phi", "Phase field order parameter");
  params.addRequiredParam<TensorInputBufferName>("laplacian_phi",
                                                 "Laplacian of phase field order parameter");
  params.addRequiredParam<std::string>("thickness", "Interface thickness");
  params.addRequiredParam<std::string>("sigma", "Interfacial tension coefficient");
  return params;
}

LBMComputeChemicalPotential::LBMComputeChemicalPotential(const InputParameters & parameters)
  : LatticeBoltzmannOperator(parameters),
    _phi(getInputBuffer("phi")),
    _laplacian_phi(getInputBuffer("laplacian_phi")),
    _D(_lb_problem.getConstant<Real>(getParam<std::string>("thickness"))),
    _sigma(_lb_problem.getConstant<Real>(getParam<std::string>("sigma")))
{
}

void
LBMComputeChemicalPotential::computeBuffer()
{
  _u = 48.0 * _sigma / _D * _phi * (_phi - 1) * (_phi - 0.5) - 3.0 * _D * _sigma * _laplacian_phi;
}
