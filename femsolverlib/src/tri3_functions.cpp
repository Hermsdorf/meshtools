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

void ComputeTRI3(double xi, double eta, double phi[3])
{
    phi[0] = xi;
    phi[1] = eta;
    phi[2] = 1.0 - xi - eta;
}

void ComputeTRI3Dx(double Dx[3][2])
{
    Dx[0][0] =  1.0; // dphi_1/dxi
    Dx[0][1] =  0.0; // dphi_1/deta 
    Dx[1][0] =  0.0; // dphi_2/dxi
    Dx[1][1] =  1.0; // dphi_2/deta
    Dx[2][0] = -1.0; // dphi_0/dxi
    Dx[2][1] =  1.0; // dphi_0/deta
}

#define X(i) (xy[i*3+0])
#define Y(i) (xy[i*3+1])
void ComputeTRI3Functions(double *xy, double *phi[3], double dphi[3][2])
{

    double x13 = X(2) - X(0);
    double x23 = x[1] - x[2];
    double y23 = y[1] - y[2];
    double y13 = y[0] - y[2];
    double detJ = x13*y23 - x23*y13;
    double invJ[2][2];
    double Dx[3][2];
    ComputeTRI3Dx(Dx);
    invJ[0][0] =  y23/detJ;
    invJ[0][1] = -y13/detJ;
    invJ[1][0] = -x23/detJ;
    invJ[1][1] =  x13/detJ;



}





