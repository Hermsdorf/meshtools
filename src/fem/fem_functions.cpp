
#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"
#include "tensor.h"
#include "element.h"
#include "fem_functions.h"

// To learn: https://www.youtube.com/watch?v=gJzqCaOEqsA

/*
Coordinate mapping between the physical coordinate system and the reference coordinate system

    Physical coordinate system: (x, y)
    Reference coordinate system: (xi, eta)

For example QUAD4 element:
                             eta
                           |
   4. (-1, 1)    o---------|---------o 3. (1, 1)
                 |         |         |
                 |         |         |
                 |         |         |
                 |         |_______________ xi
                 |                   |
                 |                   |
                 |                   |
   1. (-1, -1)   o-------------------o 2. (1, -1)


The shape functions in reference coordinate system:
    N1 = 0.25*(1-xi)*(1-eta)
    N2 = 0.25*(1+xi)*(1-eta)
    N3 = 0.25*(1+xi)*(1+eta)
    N4 = 0.25*(1-xi)*(1+eta)

So, the function x and y mapping between the physical coordinate system and the reference coordinate system is:
    x = N1*x1 + N2*x2 + N3*x3 + N4*x4
    y = N1*y1 + N2*y2 + N3*y3 + N4*y4

For example a TRI3 element in physical coordinate system with nodes (x1, y1), (x2, y2), (x3, y3), (x4, y4) being
(0,0), (1,0), (1,1) and (0, 1).

                y
               |
               |
   4. (0, 1)   o--------------------o 3. (1, 1)
               |                    |
               |                    |
               |                    |
               |                    |
               |                    |
               |                    |
   1. (0, 0)   o--------------------o_____ x
                                      2. (1, 0)

If you replace the nodes in the reference coordinate system you get the nodes in
the physical coordinate system, for example node (xi, eta) = (-1, -1) it is the node (0, 0) in the physical coordinate
system. Replacing it:
    x1 = 0.25*(1-(-1))*(1-(-1))*0 + 0.25*(1+(-1))*(1-(-1))*1 + 0.25*(1+(-1))*(1+(-1))*0 + 0.25*(1-(-1))*(1+(-1))*1 = 0
    y1 = 0.25*(1-(-1))*(1-(-1))*0 + 0.25*(1+(-1))*(1-(-1))*0 + 0.25*(1+(-1))*(1+(-1))*1 + 0.25*(1-(-1))*(1+(-1))*1 = 0
    
The node (xi, eta) = (1, 1) which is the node (1, 1) in the physical coordinate system:
    x3 = 0.25*(1-(1))*(1-(1))*0 + 0.25*(1+(1))*(1-(1))*1 + 0.25*(1+(1))*(1+(1))*0 + 0.25*(1-(1))*(1+(1))*1 = 1
    y3 = 0.25*(1-(1))*(1-(1))*0 + 0.25*(1+(1))*(1-(1))*0 + 0.25*(1+(1))*(1+(1))*1 + 0.25*(1-(1))*(1+(1))*1 = 1


Transforming a vector from the physical coordinate system to 
the reference coordinate system:  (x, y) -> (xi, eta)

                 (Jacobian Matrix) -> Transformation Matrix
                |                 |
    | x_i |     | dx/dxi  dx/deta |  | xi_i  |
    |     |  =  |                 |  |       |
    | y_i |     | dy/dxi  dy/deta |  | eta_i |
                |                 |


    So dA = det(J)*da
    where dA is the area in the physical coordinate system
          da is the area in the reference coordinate system
          det(J) is the determinant of the Jacobian Matrix

    Them the differential term in the integral becomes:
    dx*dy = det(J)*dxi*deta

*/

FEMFunction::FEMFunction()
{ 
    
}

void FEMFunction::ComputeFunction(Element& elem, QGaussData qp)
{
    switch (elem.type())
    {
        case TRI3:
            TRI3Function(elem,qp);
            break;
        case QUAD4:
            QUAD4Function(elem,qp);
            break;
        case TET4:
            TET4Function(elem, qp);
            break;
        default:
            break;
    }
}

void FEMFunction::TET4Function(Element& elem, QGaussData qp)
{
    double dpsi[3][4];
    double J[3][3]    = {{0.0, 0.0, 0.0}, {0.0, 0.0,  0.0}, {0.0, 0.0, 0.0}};
    double Jinv[3][3] = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
    double x[4], y[4], z[4];

    _xyz(0) = 0.0;
    _xyz(1) = 0.0;
    _xyz(2) = 0.0;   
    _phi.resize(4);
    _dphi.resize(4);

    double xi   = qp.first(0);
    double eta  = qp.first(1);
    double zeta = qp.first(2);

    // shape functions
    _phi[0] = 1.0 - xi - eta - zeta; // N1
    _phi[1] = xi;                    // N2
    _phi[2] = eta;                   // N3
    _phi[3] = zeta;                  // N4
    
    // shape function derivatives
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


    // compute Jacobian
    for(int i=0; i<elem.n_nodes(); i++)
    {
        x[i] = elem.node(i)(0);
        y[i] = elem.node(i)(1);
        z[i] = elem.node(i)(2);

        _xyz(0) += x[i]*_phi[i]; // x = [N1*x1 + N2*x2 + N3*x3 + N4*x4]
        _xyz(1) += y[i]*_phi[i]; // y = [N1*y1 + N2*y2 + N3*y3 + N4*y4]
        _xyz(2) += z[i]*_phi[i]; // z = [N1*z1 + N2*z2 + N3*z3 + N4*z4]

        J[0][0] +=  x[i]*dpsi[0][i]; // dx/dxi
        J[0][1] +=  y[i]*dpsi[0][i]; // dy/dxi
        J[0][2] +=  z[i]*dpsi[0][i]; // dz/dxi
        J[1][0] +=  x[i]*dpsi[1][i]; // dx/deta
        J[1][1] +=  y[i]*dpsi[1][i]; // dy/deta
        J[1][2] +=  z[i]*dpsi[1][i]; // dz/deta
        J[2][0] +=  x[i]*dpsi[2][i]; // dx/dzeta
        J[2][1] +=  y[i]*dpsi[2][i]; // dy/dzeta
        J[2][2] +=  z[i]*dpsi[2][i]; // dz/dzeta

    }

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
        _dphi[i](0) = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i] + Jinv[0][2]*dpsi[2][i];
        _dphi[i](1) = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i] + Jinv[1][2]*dpsi[2][i];
        _dphi[i](2) = Jinv[2][0]*dpsi[0][i] + Jinv[2][1]*dpsi[1][i] + Jinv[2][2]*dpsi[2][i];
    }

    _JxW = qp.second*detJ;

    // Calculo g e G;
    _g(0) = J[0][0] + J[1][0] + J[2][0];
    _g(1) = J[0][1] + J[1][1] + J[2][1];
    _g(2) = J[0][2] + J[1][2] + J[2][2];

    _G(0,0)           = J[0][0]*J[0][0] + J[1][0]*J[1][0] + J[2][0]*J[2][0]; 
    _G(0,1) = _G(1,0) = J[0][0]*J[0][1] + J[1][0]*J[1][1] ;
    _G(0,2) = _G(2,0) = J[0][0]*J[0][2] + J[1][0]*J[1][2] ;
    _G(1,1)           = J[0][1]*J[0][1] + J[1][1]*J[1][1] + J[2][1]*J[2][1];
    _G(1,2) = _G(2,1) = J[0][1]*J[0][2] + J[1][1]*J[1][2] ;
    _G(2,2)           = J[0][2]*J[0][2] + J[1][2]*J[1][2] + J[2][2]*J[2][2];


    _dxi(0) = J[0][0];
    _dxi(1) = J[0][1];
    _dxi(2) = J[0][2];
    _deta(0)  = J[1][0];
    _deta(1)  = J[1][1];
    _deta(2)  = J[1][2];
    _dzeta(0)  = J[2][0];
    _dzeta(1)  = J[2][1];
    _dzeta(2)  = J[2][2];
}

void FEMFunction::TRI3Function(Element& elem, QGaussData qp)
{
    double dpsi[2][3];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double x[3], y[3];

    _xyz(0) = 0.0;
    _xyz(1) = 0.0;
    _xyz(2) = 0.0;   
    _phi.resize(3);
    _dphi.resize(3);

    double xi  = qp.first(0);
    double eta = qp.first(1);

    // shape function
    _phi[0] = 1.0 - xi - eta; // N1
    _phi[1] = xi;             // N2
    _phi[2] = eta;            // N3

    // shape function derivative 
    dpsi[0][0] = -1.0;  // dN1/dxi 
    dpsi[0][1] =  1.0;  // dN2/dxi 
    dpsi[0][2] =  0.0;  // dN3/dxi 
    dpsi[1][0] = -1.0;  // dN1/deta
    dpsi[1][1] =  0.0;  // dN2/deta
    dpsi[1][2] =  1.0;  // dN3/deta

    for(int i=0; i<elem.n_nodes(); i++)
    {
        x[i] = elem.node(i)(0);
        y[i] = elem.node(i)(1);
        _xyz(0) += x[i]*_phi[i]; // x = [N1*x1 + N2*x2 + N3*x3]
        _xyz(1) += y[i]*_phi[i]; // y = [N1*y1 + N2*y2 + N3*y3]

        J[0][0] +=  x[i]*dpsi[0][i]; //dx/dxi
        J[0][1] +=  y[i]*dpsi[0][i]; //dy/dxi
        J[1][0] +=  x[i]*dpsi[1][i]; //dx/deta
        J[1][1] +=  y[i]*dpsi[1][i]; //dy/deta

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

    for(int i=0; i<_dphi.size(); i++)
    {
        _dphi[i](0) = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i]; // dphi_i/dx
        _dphi[i](1) = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i]; // dphi_i/dy
    }

    _JxW = qp.second*detJ;

    _g(0) = J[0][0] + J[1][0];
    _g(1) = J[0][1] + J[1][1];

    _G(0,0)          = J[0][0]*J[0][0] + J[1][0]*J[1][0];
    _G(0,1) = _G(1,0) = J[0][0]*J[0][1] + J[1][0]*J[1][1];
    _G(1,1)          = J[0][1]*J[0][1] + J[1][1]*J[1][1];

    _dxi(0) = J[0][0];
    _dxi(1) = J[0][1];
    _deta(0)  = J[1][0];
    _deta(1)  = J[1][1];
}

void FEMFunction::QUAD4Function(Element& elem, QGaussData qp)
{
    double dpsi[2][4];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double x[4], y[4];

    _xyz(0) = 0.0;
    _xyz(1) = 0.0;
    _xyz(2) = 0.0;   
    _phi.resize(4);
    _dphi.resize(4);

    double xi  = qp.first(0);
    double eta = qp.first(1);

    // shape function
    _phi[0] = 0.25*(1.0-xi)*(1.0-eta); // N1
    _phi[1] = 0.25*(1.0+xi)*(1.0-eta); // N2
    _phi[2] = 0.25*(1.0+xi)*(1.0+eta); // N3
    _phi[3] = 0.25*(1.0-xi)*(1.0+eta); // N4

    // shape function derivative 
    dpsi[0][0] = -0.25*(1.0-eta);  // dN1/dxi
    dpsi[0][1] =  0.25*(1.0-eta);  // dN2/dxi
    dpsi[0][2] =  0.25*(1.0+eta);  // dN3/dxi
    dpsi[0][3] = -0.25*(1.0+eta);  // dN4/dxi

    dpsi[1][0] = -0.25*(1.0-xi);  // dN1/deta
    dpsi[1][1] = -0.25*(1.0+xi);  // dN2/deta
    dpsi[1][2] =  0.25*(1.0+xi);  // dN3/deta
    dpsi[1][3] =  0.25*(1.0-xi);  // dN4/deta

    for(int i=0; i<elem.n_nodes(); i++)
    {
        x[i] = elem.node(i)(0);
        y[i] = elem.node(i)(1);
        _xyz(0) += x[i]*_phi[i]; // x = [N1*x1 + N2*x2 + N3*x3 + N4*x4]
        _xyz(1) += y[i]*_phi[i]; // y = [N1*y1 + N2*y2 + N3*y3 + N4*y4]

        J[0][0] +=  x[i]*dpsi[0][i]; // dx/dxi
        J[0][1] +=  y[i]*dpsi[0][i]; // dy/dxi
        J[1][0] +=  x[i]*dpsi[1][i]; // dx/deta
        J[1][1] +=  y[i]*dpsi[1][i]; // dy/deta

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

    for(int i=0; i<_dphi.size(); i++)
    {
        _dphi[i](0) = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i]; // dphi_i/dx
        _dphi[i](1) = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i]; // dphi_i/dy
    }

    _JxW = qp.second*detJ;

    _g(0) = J[0][0] + J[1][0];
    _g(1) = J[0][1] + J[1][1];

    _G(0,0)          = J[0][0]*J[0][0] + J[1][0]*J[1][0];
    _G(0,1) = _G(1,0) = J[0][0]*J[0][1] + J[1][0]*J[1][1];
    _G(1,1)          = J[0][1]*J[0][1] + J[1][1]*J[1][1];

    _dxi(0) = J[0][0];
    _dxi(1) = J[0][1];
    _deta(0)  = J[1][0];
    _deta(1)  = J[1][1];
}
