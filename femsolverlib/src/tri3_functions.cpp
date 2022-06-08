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
using namespace std;

void TRI3Shape(double xi, double eta, double psi[3])
{
    psi[0] = 1.0 - xi - eta; // N1
    psi[1] = xi;             // N2
    psi[2] = eta;            // N3
}

void TRI3DShape(double dpsi[3][2])
{
//   | dN1/dxi   dN1/deta |
//   | dN2/dxi   dN2/deta |
//   | dN3/dxi   dN3/deta |
    dpsi[0][0] = -1.0;
    dpsi[0][1] = -1.0;
    dpsi[1][0] = 1.0;
    dpsi[1][1] = 0.0;
    dpsi[2][0] = 0.0;
    dpsi[2][1] = 1.0;

}

#define X(i) (xy[i*3+0])
#define Y(i) (xy[i*3+1])
void ComputeTRI3Functions(double *xy, double qpoints[2], double phi[3], double dphi[3][2], double *JxW)
{
    double dpsi[3][2];
    double J[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    // quadrature points
    double qp = 1.0/3.0;
    double w  = 0.5;
    
    qpoints[0] = 0.0;
    qpoints[1] = 0.0;

    TRI3Shape(qp, qp, phi);
    TRI3DShape(dpsi);
    for(int i=0; i<3; i++)
    {
        double x = X(i);
        double y = Y(i);

        qpoints[0] += phi[i]*x;
        qpoints[1] += phi[i]*y;

        J[0][0] +=  x*dpsi[i][0];
        J[0][1] +=  y*dpsi[i][0];
        J[1][0] +=  x*dpsi[i][1];
        J[1][1] +=  y*dpsi[i][1];
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

    for(int i=0; i<3; i++)
    {
        dphi[i][0] = Jinv[0][0]*dpsi[i][0] + Jinv[0][1]*dpsi[i][1];
        dphi[i][1] = Jinv[1][0]*dpsi[i][0] + Jinv[1][1]*dpsi[i][1];
    }

    *JxW = w*detJ;
}

