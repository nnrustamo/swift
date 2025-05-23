/**********************************************************************/
/*                    DO NOT MODIFY THIS HEADER                       */
/*             Swift, a Fourier spectral solver for MOOSE             */
/*                                                                    */
/*            Copyright 2024 Battelle Energy Alliance, LLC            */
/*                        ALL RIGHTS RESERVED                         */
/**********************************************************************/

#pragma once

#include "TensorPostprocessor.h"

/**
 * Compute the integral of a Tensor buffer
 */
class TensorInterfaceVelocityPostprocessor : public TensorPostprocessor
{
public:
  static InputParameters validParams();

  TensorInterfaceVelocityPostprocessor(const InputParameters & parameters);

  virtual void initialize() override {}
  virtual void execute() override;
  virtual void finalize() override {}
  virtual PostprocessorValue getValue() const override;

protected:
  const std::vector<torch::Tensor> & _u_old;
  unsigned int _dim;
  Real _velocity;

  /// imaginary unit i
  const torch::Tensor _i;

  const Real _gradient_threshold;
};
