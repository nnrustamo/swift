/**********************************************************************/
/*                    DO NOT MODIFY THIS HEADER                       */
/*             Swift, a Fourier spectral solver for MOOSE             */
/*                                                                    */
/*            Copyright 2024 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#pragma once

#ifdef NEML2_ENABLED

#include "TensorProblem.h"


class LBMStencilBase;
class LatticeBoltzmannMesh;

/**
 * Problem object for solving lattice Boltzmann problems
 */
class LatticeBoltzmannProblem : public TensorProblem
{
public:
  static InputParameters validParams();

  LatticeBoltzmannProblem(const InputParameters & parameters);

  void init() override;

  void execute(const ExecFlagType & exec_type) override;
  
  // void advanceState() override;

  void addStencil(const std::string & stencil_name,
                              const std::string & name,
                              InputParameters & parameters);
  
  const LBMStencilBase & getStencil() const {return *_stencil; }

  const bool & isSlipEnabled() const {return _enable_slip; }

  const torch::Tensor & getSlipRelaxationMatrix() const {return _slip_relaxation_matrix;}

  const int & getTotalSteps() const {return _t_total;}

  /// sets up slip model
  void enableSlipModel();

  /// sets convergence residual
  void setSolverResidual(const Real & residual);

  /// sets tensor to a value at wall nodes
  void setTensorToValue(torch::Tensor & t, const Real & value);

protected:
  /// LBM Mesh object
  LatticeBoltzmannMesh * _lbm_mesh;

  /// LBM stencils
  std::shared_ptr<LBMStencilBase> _stencil;

  /// buffers with extra dimension
  std::map<std::string, unsigned int> _buffer_extra_dimension;

  /// enables slip models
  bool _enable_slip;

  /// mean free path
  Real _mfp;

  /// resolution
  Real _dx;

  // slip coefficient
  const Real _A = 0.6;

  /// relaxation matrix as a funcion of Kn and local pore size in slip model
  torch::Tensor _slip_relaxation_matrix;

  /// used to restricts construction of more than one stencil object
  unsigned int _stencil_counter = 0;

  /// convergence residual
  Real _convergence_residual;

  /// total number of time steps taken
  int _t_total = 0;

public:
  /// LBM constants
  const Real _cs = 1.0 / sqrt(3.0);
  const Real _cs2 = _cs * _cs;
  const Real _cs4 = _cs2 * _cs2;

};

#endif