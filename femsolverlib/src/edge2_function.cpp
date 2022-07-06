//
//  Edge Linear Function
//  (0,0)     (1,0)
//   +----------+
//
//

void QGaussEdge2(int *nqp, double qp[][2], double *qw)
{
  //
  //  QGaussEdge2(nqp, qp, qw)
  //
  //  Purpose:
  //  -------
  //  This function computes the quadrature points and weights for a
  //  linear edge.
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
  *nqp  = 1;
  qp[0][0] = 0.0;
  qw[0] = 2.0;
}

void EDGE2Shape(double _xi[], double psi[])
{
  double xi  = _xi[0];
  psi[0] = 0.5*(1.0-xi); // N1
  psi[1] = 0.5*(1.0+xi); // N2
}

void EDGE2DShape(double _xi[], double dpsi[][2])
{
  double xi  = _xi[0];
  dpsi[0][0] = -0.5; // dN1/dxi
  dpsi[0][2] =  0.5; // dN2/dxi
}


// #define X(i) (coords[i*3+0])
// #define Y(i) (coords[i*3+1])
// #define Z(i) (coords[i*3+2])
// void ComputeEdge2Functions(double gp[], double qw, double *coords, double phi[2], double dphi[1][2], double *JxW)
// {
//     double dpsi[1][2];
//     double J         = 0.0;
//     double Jinv      = 0.0;
    
//     EDGE2Shape (gp, phi);
//     EDGE2DShape(gp,dpsi);
//     for(int i=0; i<2; i++)
//     {
//         double x = X(i);

//         J   +=  x*dpsi[0][i];
//     }

//     if(J < 0.0)
//     {
//         std::cout << "Error: detJ < 0.0\n" << std::endl;
//         exit(1);
//     }

//     double invJ = 1.0/J;

//     for(int i=0; i<2; i++)
//     {
//         dphi[i][0] = Jinv*dpsi[0][i] + Jinv[0][1]*dpsi[1][i];
//         dphi[i][1] = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i];
//     }

//     *JxW = qw*detJ;
// }

