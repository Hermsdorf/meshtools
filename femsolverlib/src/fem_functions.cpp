
#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"
#include "tensor.h"


#include "fem_functions.h"


void TRI3DefaultQGauss(std::vector<RealVector> &points, std::vector<double> &qw);

void TRI3ComputeFunctions( RealVector q_point, double qw, std::vector<Point> coords, RealVector &p_gauss, 
                           std::vector<double>   &phi, 
                           std::vector<Gradient> &dphi ,
                           double &JxW);

void TRI3Stab(RealVector q_point, std::vector<Point> coords, RealVector &g,  RealTensor &G);

void QUAD4DefaultQGauss(std::vector<Point> &qpoints, std::vector<double> &qw);

void QUAD4ComputeFunctions(RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW);

void QUAD4Stab(RealVector q_point, std::vector<Point> coords, RealVector &g,  RealTensor &G);

void TET4DefaultQGauss(std::vector<Point> &qpoints, std::vector<double> &qw);

void TET4ComputeFunctions(RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW);
 

void FEMGetQGauss(MeshElementType elem_type, std::vector<RealVector> &points, std::vector<double> &qw)
{
    switch (elem_type)
    {
    case TRI3:
        TRI3DefaultQGauss(points,qw);
        break;
    case QUAD4:
        QUAD4DefaultQGauss(points,qw);
        break;
    case TET4:
        TET4DefaultQGauss(points,qw);
        break;
    default:
        throw std::runtime_error("Element type not supported");
        break;
    }
}


void FEMComputeFunctions(MeshElementType elem_type, RealVector qp, double qw, std::vector<Point> coords, RealVector &point, 
                           std::vector<double> &phi, std::vector<Gradient> &dphi, double &JxW)
{
    switch (elem_type)
    {
    case TRI3:
        TRI3ComputeFunctions(qp, qw,coords, point, phi,dphi,JxW);
        break;
    case QUAD4:
        QUAD4ComputeFunctions(qp, qw,coords, point, phi,dphi,JxW);
        break;
    case TET4:
        TET4ComputeFunctions(qp, qw,coords, point, phi,dphi,JxW);
        break;
    default:
         std::cout <<" Elemento: " << elem_type << std::endl;
         throw std::runtime_error("Element type not supported");
        break;
    }
}


void FEMStab(MeshElementType elem_type, RealVector qp, std::vector<Point> &coords, RealVector &g,  RealTensor &G)
{
    switch (elem_type)
    {
    case TRI3:
        TRI3Stab(qp, coords,g,G);
        break;
    case QUAD4:
        QUAD4Stab(qp,coords,g,G);
        break;
    default:
         std::cout <<" Elemento: " << elem_type << std::endl;
         throw std::runtime_error("Element type not supported");
        break;
    }
}


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

        _xyz(0) += x[i]*_phi[i];
        _xyz(1) += y[i]*_phi[i];
        _xyz(2) += z[i]*_phi[i];

        J[0][0] +=  x[i]*dpsi[0][i]; // dxi/dx
        J[0][1] +=  y[i]*dpsi[0][i]; // dxi/dy
        J[0][2] +=  z[i]*dpsi[0][i]; // dxi/dz
        J[1][0] +=  x[i]*dpsi[1][i]; // deta/dx
        J[1][1] +=  y[i]*dpsi[1][i]; // deta/dy
        J[1][2] +=  z[i]*dpsi[1][i]; // deta/dz
        J[2][0] +=  x[i]*dpsi[2][i]; // dzeta/dx
        J[2][1] +=  y[i]*dpsi[2][i]; // dzeta/dy
        J[2][2] +=  z[i]*dpsi[2][i]; // dzeta/dz

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
        _dphi[i](0) = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i] + Jinv[0][2]*dpsi[2][i];
        _dphi[i](1) = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i] + Jinv[1][2]*dpsi[2][i];
        _dphi[i](2) = Jinv[2][0]*dpsi[0][i] + Jinv[2][1]*dpsi[1][i] + Jinv[2][2]*dpsi[2][i];
    }

    _JxW = qp.second*detJ;
}

void FEMFunction::TRI3Function(Element& elem, QGaussData qp)
{
    double dpsi[2][3];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double x[3], y[3];
   
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
        _xyz(0) += x[i]*_phi[i];
        _xyz(1) += y[i]*_phi[i];

        J[0][0] +=  x[i]*dpsi[0][i]; //dxidx
        J[0][1] +=  y[i]*dpsi[0][i]; //dxidy
        J[1][0] +=  x[i]*dpsi[1][i]; //detadx
        J[1][1] +=  y[i]*dpsi[1][i]; //detady

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
   
}

void FEMFunction::QUAD4Function(Element& elem, QGaussData qp)
{
    double dpsi[2][4];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double x[4], y[4];
    
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
        _xyz(0) += x[i]*_phi[i];
        _xyz(1) += y[i]*_phi[i];

        J[0][0] +=  x[i]*dpsi[0][i]; //dxidx
        J[0][1] +=  y[i]*dpsi[0][i]; //dxidy
        J[1][0] +=  x[i]*dpsi[1][i]; //detadx
        J[1][1] +=  y[i]*dpsi[1][i]; //detady

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
   
}

/*
void FEMFunction::TRI3FaceFunction(Element& elem, QGaussData qp)
{
    double dpsi[2][3];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double x[3], y[3];
   
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
        z[i] = elem.node(i)(2);

        _xyz(0) += x[i]*_phi[i];
        _xyz(1) += y[i]*_phi[i];
        _xyz(2) += z[i]*_phi[i];

        J[0][0] +=  x[i]*dpsi[0][i]; //dxidx
        J[0][1] +=  y[i]*dpsi[0][i]; //dxidy
        J[0][2] +=  z[i]*dpsi[0][i]; //dxidz

        J[1][0] +=  x[i]*dpsi[1][i]; //detadx
        J[1][1] +=  y[i]*dpsi[1][i]; //detady
        J[1][2] +=  z[i]*dpsi[1][i]; //detadz

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
   
}
*/
