/**********************************************************************/
/*                    DO NOT MODIFY THIS HEADER                       */
/*             Swift, a Fourier spectral solver for MOOSE             */
/*                                                                    */
/*            Copyright 2024 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#include "LBMMacroscopicDirichletBC.h"

using namespace torch::indexing;

registerMooseObject("SwiftApp", LBMMacroscopicDirichletBC);

InputParameters
LBMMacroscopicDirichletBC::validParams()
{
  InputParameters params = LBMBoundaryCondition::validParams();
  params.addClassDescription("LBMMacroscopicDirichletBC object");
  params.addParam<std::string>("value",
                               "0.0"
                               "Value at the boundary");
  return params;
}

LBMMacroscopicDirichletBC::LBMMacroscopicDirichletBC(const InputParameters & parameters)
  : LBMBoundaryCondition(parameters),
    _value(_lb_problem.getConstant<Real>(getParam<std::string>("value")))
{
}

void
LBMMacroscopicDirichletBC::bottomBoundary()
{
  // along y direction at y = 0
  _u.index_put_({Slice(), 0, Slice()}, _value);
}

void
LBMMacroscopicDirichletBC::topBoundary()
{
  // along y direction at y = N_y - 1
  _u.index_put_({Slice(), _grid_size[1] - 1, Slice()}, _value);
}

void
LBMMacroscopicDirichletBC::leftBoundary()
{
  // along x direction at x = 0
  _u.index_put_({0, Slice(), Slice()}, _value);
}

void
LBMMacroscopicDirichletBC::rightBoundary()
{
  // along x direction at x = N_x - 1
  _u.index_put_({_grid_size[0] - 1, Slice(), Slice()}, _value);
}

void
LBMMacroscopicDirichletBC::frontBoundary()
{
  // along z direction at z = 0
  _u.index_put_({Slice(), Slice(), 0}, _value);
}

void
LBMMacroscopicDirichletBC::backBoundary()
{
  // along z direction at z = N_z-1
  _u.index_put_({Slice(), Slice(), _grid_size[2] - 1}, _value);
}

void
LBMMacroscopicDirichletBC::wallBoundary()
{
  _u.index_put_({_boundary_indices.index({Slice(), 0}),
                 _boundary_indices.index({Slice(), 1}),
                 _boundary_indices.index({Slice(), 2})},
                _value);
}
