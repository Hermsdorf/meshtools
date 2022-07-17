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
using namespace std;

void QGaussTri3(int nqp, double qp[][2], double qw[])

  //
  //  QGaussTri3(nqp, qp, qw)
  //
  //  Purpose:
  //  -------
  //  This function computes the quadrature points and weights for a
  //  triangular element.
  //
  //  Parameters:
  //  -----------
  //  nqp    - number of quadrature points
  //  qp     - quadrature points
  //  qw     - quadrature weights
  //
  //  Return value:
  //  -------------
  //  None
  //
  {
     assert(nqp==1);
     qp[0][0] = 1.0/3.0;
     qp[0][1] = 1.0/3.0;
     qw[0] = 0.5;
  }

void TRI3Shape(double _xi[], double psi[])
{
    double xi  = _xi[0];
    double eta = _xi[1];
    psi[0] = 1.0 - xi - eta; // N1
    psi[1] = xi;             // N2
    psi[2] = eta;            // N3
}

void TRI3DShape(double _xi[], double dpsi[][3])
{

    dpsi[0][0] = -1.0;  // dN1/dxi 
    dpsi[0][1] =  1.0;  // dN2/dxi 
    dpsi[0][2] =  0.0;  // dN3/dxi 
    dpsi[1][0] = -1.0;  // dN1/deta
    dpsi[1][1] =  0.0;  // dN2/deta
    dpsi[1][2] =  1.0;  // dN3/deta

}

#define X(i) (coords[i*3+0])
#define Y(i) (coords[i*3+1])
void ComputeTRI3Functions(double gp[], double qw, double *coords, double xyqp[2], double phi[3], double dphi[3][2], double *JxW)
{
    double dpsi[2][3];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
   
    TRI3Shape(gp, phi);
    TRI3DShape(gp,dpsi);
    for(int i=0; i<3; i++)
    {
        double x = X(i);
        double y = Y(i);
        xyqp[0] += x*phi[i];
        xyqp[1] += y*phi[i];

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

    for(int i=0; i<3; i++)
    {
        dphi[i][0] = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i];
        dphi[i][1] = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i];
    }

    *JxW = qw*detJ;
}
