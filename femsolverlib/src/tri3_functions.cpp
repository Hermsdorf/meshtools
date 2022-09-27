//  Triangular 3-node element functions
//  (0,1)
//  +
//  | \
//  |  \
//  |   \
//  |    \
//  |     \ 
//  |      \    
//  |       \   
//  |        \  
//  |         \ 
//  +----------+  
// (0,0)       (1,0)
//
//
#include <iostream>
#include <cassert>
#include <vector>
using namespace std;

#include "numeric_vector.h"


//  This function computes the quadrature points and weights for a
//  triangular element.
//
//  @param nqp    number of quadrature points
//  @param qp     quadrature points
//  @param qw     quadrature weights
//
//  @return None 
//
void TRI3DefaultQGauss(std::vector<Point> &qpoints, std::vector<double> &qw)
  {
     qpoints.resize(1);
     qw.resize(1);
     qpoints[0](0) = 1.0/3.0;
     qpoints[0](1) = 1.0/3.0;
     qpoints[0](2) = 0.0;
     qw[0] = 0.5;
  }

void TRI3Shape(Point _xi, std::vector<double> & psi)
{
    double xi  = _xi(0);
    double eta = _xi(1);
    psi[0] = 1.0 - xi - eta; // N1
    psi[1] = xi;             // N2
    psi[2] = eta;            // N3
}

void TRI3DShape(Point _xi, double dpsi[][3])
{
    dpsi[0][0] = -1.0;  // dN1/dxi 
    dpsi[0][1] =  1.0;  // dN2/dxi 
    dpsi[0][2] =  0.0;  // dN3/dxi 
    dpsi[1][0] = -1.0;  // dN1/deta
    dpsi[1][1] =  0.0;  // dN2/deta
    dpsi[1][2] =  1.0;  // dN3/deta

}

#define X(i) (coords[i](0))
#define Y(i) (coords[i](1))
void TRI3ComputeFunctions( RealVector q_point, double qw, std::vector<Point> coords, RealVector &p_gauss, 
                           std::vector<double>   &phi, 
                           std::vector<Gradient> &dphi ,
                           double &JxW)
{
    double dpsi[2][3];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double x[3], y[3];
   
    TRI3Shape(q_point, phi);
    TRI3DShape(q_point,dpsi);
    for(int i=0; i<3; i++)
    {
        x[i] = X(i);
        y[i] = Y(i);
        p_gauss(0) += x[i]*phi[i];
        p_gauss(1) += y[i]*phi[i];

        J[0][0] +=  x[i]*dpsi[0][i];
        J[0][1] +=  y[i]*dpsi[0][i];
        J[1][0] +=  x[i]*dpsi[1][i];
        J[1][1] +=  y[i]*dpsi[1][i];
    }

    double detJ = J[0][0]*J[1][1] - J[0][1]*J[1][0];
    if(detJ < 0.0)
    {
        std::cout << "Error: detJ < 0.0  -- det = " << detJ << std::endl;
        exit(1);
    }

    double invdetJ = 1.0/detJ;
    Jinv[0][0] =  J[1][1]*invdetJ;
    Jinv[0][1] = -J[0][1]*invdetJ;
    Jinv[1][0] = -J[1][0]*invdetJ;
    Jinv[1][1] =  J[0][0]*invdetJ;

    for(int i=0; i<dphi.size(); i++)
    {
        dphi[i](0) = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i]; // dphi_i/dx
        dphi[i](1) = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i]; // dphi_i/dy
    }
    JxW = qw*detJ;
}

/*
void TRI3Stab( RealVector q_point, std::vector<Point> coords, NumericVector<double> &g,  DenseMatrix<double> &G, double &JxW)
{
    double dpsi[2][3];
    double dxidx = 0.0, dxidy=0.0;
    double detadx = 0.0, detady=0.0;
    double x[3], y[3];
   
    TRI3DShape(q_point,dpsi);
    for(int i=0; i<3; i++)
    {
        x[i] = X(i);
        y[i] = Y(i);

        dxidx +=  x[i]*dpsi[0][i]; // dxi/dx
        dxidy +=  y[i]*dpsi[0][i]; // dxi/dy
        detadx +=  x[i]*dpsi[1][i]; // deta/dx
        detady +=  y[i]*dpsi[1][i]; // deta/dy
    }

    g(0) = dxidx + detadx;
    g(1) = dxidy + detady;

    G(0,0) = dxidx*dxidx + detadx*detadx;
    G(0,1) = G(1,0) = dxidx*dxidy + detadx*detady;
    G(1,1) = dxidy*dxidy + detady*detady;
   
}
*/
