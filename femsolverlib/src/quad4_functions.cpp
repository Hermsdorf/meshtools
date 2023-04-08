
#include <iostream>
#include <vector>
#include "numeric_vector.h"
#include "tensor.h"

//  Linear Quadrilateral element functions
// (-1,1)     (1,1)
//  +----------+
//  |          |
//  |          |
//  |          |
//  |          |
//  +----------+  
// (-1,-1)    (1,-1)
//
//

/**
 *  This function computes the quadrature points and weights for a
 *  quadrilateral element.
 *
 *  @param[out] qpoints  quadrature points
 *  @param[out] qw  quadrature weights
 *
 *  @return None
 */
void QUAD4DefaultQGauss(std::vector<Point> &qpoints, std::vector<double> &qw)
{
  qpoints.resize(4);
  qw.resize(4);
  qpoints[0](0) =  -0.57735026919;
  qpoints[0](1) =  -0.57735026919;
  qpoints[1](0) =   0.57735026919;
  qpoints[1](1) =  -0.57735026919;
  qpoints[2](0) =   0.57735026919;
  qpoints[2](1) =   0.57735026919;
  qpoints[3](0) =  -0.57735026919;
  qpoints[3](1) =   0.57735026919;
  qw[0] = qw[1] = qw[2] = qw[3] = 1.0;
}

/**
 * This function computes the shape functions at quadrature point.
 * @param[in] _xi   quadrature point
 * @param[out] psi   shape functions
 *
 * @return None
 */
void QUAD4Shape(Point _xi, std::vector<double> & psi)
{
    double xi  = _xi(0);
    double eta = _xi(1);
    psi[0] = 0.25*(1.0-xi)*(1.0-eta); // N1
    psi[1] = 0.25*(1.0+xi)*(1.0-eta); // N2
    psi[2] = 0.25*(1.0+xi)*(1.0+eta); // N3
    psi[3] = 0.25*(1.0-xi)*(1.0+eta); // N4
} 

/**
 * This function computes the shape function derivatives at quadrature point.
 * @param[in] _xi   quadrature point
 * @param[out] dpsi   shape function derivatives
 *
 * @return None
 */
void QUAD4DShape(Point _xi, double dpsi[][4])
{
    double xi  = _xi(0);
    double eta = _xi(1);

    dpsi[0][0] = -0.25*(1.0-eta);  // dN1/dxi
    dpsi[0][1] =  0.25*(1.0-eta);  // dN2/dxi
    dpsi[0][2] =  0.25*(1.0+eta);  // dN3/dxi
    dpsi[0][3] = -0.25*(1.0+eta);  // dN4/dxi

    dpsi[1][0] = -0.25*(1.0-xi);  // dN1/deta
    dpsi[1][1] = -0.25*(1.0+xi);  // dN2/deta
    dpsi[1][2] =  0.25*(1.0+xi);  // dN3/deta
    dpsi[1][3] =  0.25*(1.0-xi);  // dN4/deta

}

#define X(i) (coords[i](0))
#define Y(i) (coords[i](1))

/**
 * This function computes the shape functions, shape function derivatives
 * and Jacobian at quadrature point.
 * @param[in] qp   quadrature point
 * @param[in] qw   quadrature weight
 * @param[in] coords   nodal coordinates
 * @param[out] point   quadrature point
 * @param[out] phi   shape functions
 * @param[out] dphi   shape function derivatives
 * @param[out] JxW   Jacobian
 *
 * @return None
 */
void QUAD4ComputeFunctions(RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW)
{
    double dpsi[2][4];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    
    QUAD4Shape(qp, phi);
    QUAD4DShape(qp,dpsi);

    point(0) = 0.0;
    point(1) = 0.0;
    for(int i=0; i<4; i++)
    {
        double x = X(i);
        double y = Y(i);

        point(0) += x*phi[i];
        point(1) += y*phi[i];

        J[0][0] +=  x*dpsi[0][i];
        J[0][1] +=  y*dpsi[0][i];
        J[1][0] +=  x*dpsi[1][i];
        J[1][1] +=  y*dpsi[1][i];
    }

    double detJ = J[0][0]*J[1][1] - J[0][1]*J[1][0];
    if(detJ < 0.0)
    {
        std::cout << "Error: detJ < 0.0\n" << std::endl;
        exit(1);
    }

    double invdetJ = 1.0/detJ;
    Jinv[0][0] =  J[1][1]*invdetJ;
    Jinv[0][1] = -J[0][1]*invdetJ;
    Jinv[1][0] = -J[1][0]*invdetJ;
    Jinv[1][1] =  J[0][0]*invdetJ;

    for(int i=0; i<4; i++)
    {
        dphi[i](0) = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i];
        dphi[i](1) = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i];
    }

    JxW = qw*detJ;
}


void QUAD4Stab(RealVector q_point, std::vector<Point> coords, RealVector &g,  RealTensor &G)
{
    double dpsi[2][4];
    double dxidx  = 0.0, dxidy =0.0;
    double detadx = 0.0, detady=0.0;
    double x[3], y[3];
   
    QUAD4DShape(q_point,dpsi);
    for(int i=0; i<coords.size(); i++)
    {
        x[i] = X(i);
        y[i] = Y(i);

        dxidx  +=  x[i]*dpsi[0][i];  // dxi/dx
        dxidy  +=  y[i]*dpsi[0][i];  // dxi/dy
        detadx +=  x[i]*dpsi[1][i];  // deta/dx
        detady +=  y[i]*dpsi[1][i];  // deta/dy
    }

    g(0) = dxidx + detadx;
    g(1) = dxidy + detady;

    G(0,0)          = dxidx*dxidx + detadx*detadx;
    G(0,1) = G(1,0) = dxidx*dxidy + detadx*detady;
    G(1,1)          = dxidy*dxidy + detady*detady;
   
}
