#ifndef FEM_FUNCTIONS_H
#define FEM_FUNCTIONS_H

#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"
#include "tensor.h" 



void FEMGetQGauss(MeshElementType elem_type, std::vector<RealVector> &points, std::vector<double> &qw);


void FEMComputeFunctions(MeshElementType ele_type, RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW);

void FEMStab(MeshElementType elem_type, RealVector qp, std::vector<Point> &coords, RealVector &g,  RealTensor &G);

#endif /* FEM_FUNCTIONS_H */
