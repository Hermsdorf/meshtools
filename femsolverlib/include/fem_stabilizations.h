#ifndef FEM_STABILIZATIONS_H
#define FEM_STABILIZATIONS_H

#include "numeric_vector.h"
#include "tensor.h"

// CAU stabilization parameter
double CAUStab(double u, double u_old, Gradient grad_u, double f,
               RealVector velocity, double sigma, double K,
               double dt, double h_caract);

// SUPG stabilization parameter
double TAUStab(RealVector velocity, RealTensor G, double k, double dt_stab=1.0,
               double dt=1.0);

#endif // FEM_STABILIZATIONS_H