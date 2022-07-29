#ifndef FEM_FUNCTIONS_H
#define FEM_FUNCTIONS_H

#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"


void TRI3DefaultQGauss(std::vector<RealVector> &points, std::vector<double> &qw);

void TRI3ComputeFunctions( RealVector q_point, double qw, std::vector<Point> coords, RealVector &p_gauss, 
                           std::vector<double>   &phi, 
                           std::vector<Gradient> &dphi ,
                           double &JxW);

void QUAD4DefaultQGauss(std::vector<Point> &qpoints, std::vector<double> &qw);

void QUAD4ComputeFunctions(RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW);
 
#endif /* FEM_FUNCTIONS_H */
