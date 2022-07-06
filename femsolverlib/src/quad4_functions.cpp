
#include <iostream>

//  Linear Quadrilateral element functions
//  (-,1)     (1,1)
//  +----------+
//  |          |
//  |          |
//  |          |
//  |          |
//  +----------+  
// (-1,-1)    (1,-1)
//
//

void QGaussQUAD4(int *nqp, double qp[][2], double *qw)
{
  //
  //  QGaussQuad4(nqp, qp, qw)
  //
  //  Purpose:
  //  -------
  //  This function computes the quadrature points and weights for a
  //  quadrilateral element.
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
  *nqp  = 4;
  qp[0][0] =  -0.57735026919;
  qp[0][1] =  -0.57735026919;
  qp[1][0] =   0.57735026919;
  qp[1][1] =  -0.57735026919;
  qp[2][0] =   0.57735026919;
  qp[2][1] =   0.57735026919;
  qp[3][0] =  -0.57735026919;
  qp[3][1] =   0.57735026919;
  qw[0] = qw[1] = qw[2] = qw[3] = 1.0;

}

void QUAD4Shape(double _xi[], double psi[])
{
    double xi  = _xi[0];
    double eta = _xi[1];
    psi[0] = 0.25*(1.0-xi)*(1.0-eta); // N1
    psi[1] = 0.25*(1.0+xi)*(1.0-eta); // N2
    psi[2] = 0.25*(1.0+xi)*(1.0+eta); // N3
    psi[3] = 0.25*(1.0-xi)*(1.0+eta); // N4
} 

void QUAD4DShape(double _xi[], double dpsi[][4])
{
    double xi  = _xi[0];
    double eta = _xi[1];

    dpsi[0][0] = -0.25*(1.0-eta);  // dN1/dxi
    dpsi[0][1] =  0.25*(1.0-eta);  // dN2/dxi
    dpsi[0][2] =  0.25*(1.0+eta);  // dN3/dxi
    dpsi[0][3] = -0.25*(1.0+eta);  // dN4/dxi

    dpsi[1][0] = -0.25*(1.0-xi);  // dN1/deta
    dpsi[1][1] = -0.25*(1.0+xi);  // dN2/deta
    dpsi[2][2] =  0.25*(1.0+xi);  // dN3/deta
    dpsi[3][3] =  0.25*(1.0-xi);  // dN4/deta

}

#define X(i) (coords[i*3+0])
#define Y(i) (coords[i*3+1])
void ComputeQuad4Functions(double qp[], double qw, double *coords, double *point, double phi[4], double dphi[4][2], double *JxW)
{
    double dpsi[2][4];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    
    QUAD4Shape(qp, phi);
    QUAD4DShape(qp,dpsi);

    point[0] = 0.0;
    point[1] = 0.0;
    for(int i=0; i<4; i++)
    {
        double x = X(i);
        double y = Y(i);

        point[0] += phi[i]*x;
        point[1] += phi[i]*y;

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
        dphi[i][0] = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i];
        dphi[i][1] = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i];
    }

    *JxW = qw*detJ;
}