
#include <iostream>
#include <vector>
#include "numeric_vector.h"

//  Linear Tetrahedral element functions
// 
//   *
//   |\
//   | \
//   |  \
//   |   *
//   |  / 
//   | /           
//   *-------------*
// (-1,-1)    (1,-1)
//
//

//  This function computes the quadrature points and weights for a
//  quadrilateral element.
//
//  @param nqp    number of quadrature points
//  @param qp     quadrature points
//  @param qw     quadrature weights
//
//  @return None
//
void TET4DefaultQGauss(std::vector<Point> &qpoints, std::vector<double> &qw)
{
  qpoints.resize(1);
  qw.resize(1);
  qpoints[0](0) =  0.25;
  qpoints[0](1) =  0.25;
  qw[0] = 1;0/6.0;

}

void TET4Shape(Point _xi, std::vector<double> & psi)
{
    double xi   = _xi(0);
    double eta  = _xi(1);
    double zeta = _xi(2);
    psi[0] = 1.0 - xi - eta - zeta; // N1
    psi[1] = xi;                    // N2
    psi[2] = eta;                   // N3
    psi[3] = zeta;                  // N4


} 

void TET4DShape(Point _xi, double dpsi[][4])
{
    double xi   = _xi(0);
    double eta  = _xi(1);
    double zeta = _xi(2);

    dpsi[0][0] = -1.0;  // dN1/dxi
    dpsi[0][1] =  1.0;  // dN2/dxi
    dpsi[0][2] =  0.0;  // dN3/dxi
    dpsi[0][3] =  0.0;  // dN4/dxi

    dpsi[1][0] = -1.0;  // dN1/deta
    dpsi[1][1] =  0.0;  // dN2/deta
    dpsi[1][2] =  1.0;  // dN3/deta
    dpsi[1][3] =  0.0;  // dN4/deta

    dpsi[2][0] = -1.0;  // dN1/dzeta
    dpsi[2][1] =  0.0;  // dN2/dzeta
    dpsi[2][2] =  0.0;  // dN3/dzeta
    dpsi[2][3] =  1.0;  // dN4/dzeta

}

#define X(i) (coords[i](0))
#define Y(i) (coords[i](1))
#define Z(i) (coords[i](2))

void TET4ComputeFunctions(RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW)
{
    double dpsi[3][4];
    double J[3][3]    = {{0.0, 0.0, 0.0}, {0.0, 0.0,  0.0}, {0.0, 0.0, 0.0}};
    double Jinv[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
    
    TET4Shape(qp, phi);
    TET4DShape(qp,dpsi);

    point(0) = 0.0;
    point(1) = 0.0;
    point(2) = 0.0;
    for(int i=0; i<4; i++)
    {
        double x = X(i);
        double y = Y(i);
        double z = Z(i);

        point(0) += x*phi[i];
        point(1) += y*phi[i];
        point(2) += z*phi[i];

        J[0][0] +=  x*dpsi[0][i];
        J[0][1] +=  y*dpsi[0][i];
        J[0][2] +=  z*dpsi[0][i];
        J[1][0] +=  x*dpsi[1][i];
        J[1][1] +=  y*dpsi[1][i];
        J[1][2] +=  z*dpsi[1][i];
        J[2][0] +=  x*dpsi[2][i];
        J[2][1] +=  y*dpsi[2][i];
        J[2][2] +=  z*dpsi[2][i];
    }

    // TODO: check if this is correct
    double detJ = J[0][0]*(J[1][1]*J[2][2]-J[1][2]*J[2][1]) - J[0][1]*(J[1][0]*J[2][2]-J[1][2]*J[2][0]) + J[0][2]*(J[1][0]*J[2][1]-J[1][1]*J[2][0]);
    if(detJ < 0.0)
    {
        std::cout << "Error: detJ < 0.0\n" << std::endl;
        exit(1);
    }

    double invdetJ = 1.0/detJ;
    Jinv[0][0] = (J[1][1]*J[2][2]-J[1][2]*J[2][1])*invdetJ;
    Jinv[0][1] = (J[0][2]*J[2][1]-J[0][1]*J[2][2])*invdetJ;
    Jinv[0][2] = (J[0][1]*J[1][2]-J[0][2]*J[1][1])*invdetJ;
    Jinv[1][0] = (J[1][2]*J[2][0]-J[1][0]*J[2][2])*invdetJ;
    Jinv[1][1] = (J[0][0]*J[2][2]-J[0][2]*J[2][0])*invdetJ;
    Jinv[1][2] = (J[0][2]*J[1][0]-J[0][0]*J[1][2])*invdetJ;
    Jinv[2][0] = (J[1][0]*J[2][1]-J[1][1]*J[2][0])*invdetJ;
    Jinv[2][1] = (J[0][1]*J[2][0]-J[0][0]*J[2][1])*invdetJ;
    Jinv[2][2] = (J[0][0]*J[1][1]-J[0][1]*J[1][0])*invdetJ;

    for(int i=0; i<4; i++)
    {
        dphi[i](0) = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i] + Jinv[0][2]*dpsi[2][i];
        dphi[i](1) = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i] + Jinv[1][2]*dpsi[2][i];
        dphi[i](2) = Jinv[2][0]*dpsi[0][i] + Jinv[2][1]*dpsi[1][i] + Jinv[2][2]*dpsi[2][i];
    }

    JxW = qw*detJ;
}