#ifndef FEM_FUNCTIONS_H
#define FEM_FUNCTIONS_H

#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"


void FEMGetQGauss(MeshElementType elem_type, std::vector<RealVector> &points, std::vector<double> &qw);


void FEMComputeFunctions(MeshElementType ele_type, RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW);



#endif /* FEM_FUNCTIONS_H */
