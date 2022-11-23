
#include  "boundary_fem_functions.h"


unsigned int cyc3[5]={0, 1, 2, 0, 1 };

BoundaryFEMFunction::BoundaryFEMFunction()
{

}

void BoundaryFEMFunction::ComputeFunction(Element& elem, QGaussData qp)
{
    switch (elem.type())
    {
    case EDGE2:
        EDGEFaceFunction(elem, qp);
        break;
    case TRI3:
        TRI3FaceFunction(elem,qp);
        break;
    case QUAD4:
        QUAD4FaceFunction(elem,qp);
        break;
    default:
        break;
    }
}

void BoundaryFEMFunction::EDGEFaceFunction(Element& elem, QGaussData qp)
{
    
    _phi.resize(2);
    _dphi.resize(2);
    double x[2], y[2];
    double dpsi[1][2];
    double J[1][2]    = {{0.0, 0.0}};

    double xi = qp.first(0);

    // EDGE2 shape functions
    _phi[0] = 0.5*(1-xi); // N1
    _phi[1] = 0.5*(1+xi); // N2

    // EDGE2 shape functions derivatives
    dpsi[0][0] = -0.5;  //dN1/dxi
    dpsi[0][1] =  0.5;  //dN2/dxi

    // compute x, dxdxi at the quadrature points

    for(int i=0; i<elem.n_nodes(); i++)
    {
        x[i] = elem.node(i)(0);
        y[i] = elem.node(i)(1);
        _xyz(0) += x[i]*_phi[i];
        _xyz(1) += y[i]*_phi[i];

        J[0][0] += x[i]*dpsi[0][i]; //dxidx
        J[0][1] += y[i]*dpsi[0][i]; //dxidy
    }

    // compute the determinant of the Jacobian
    double jac = sqrt(J[0][0]*J[0][0] + J[0][1]*J[0][1]);

    // compute the inverse of the Jacobian
    double invjac = 1.0/jac;

    // for(int i=0; i<_dphi.size(); i++)
    // {
    //     _dphi[i](0) = Jinv[0][0]*dpsi[0][i] + Jinv[0][1]*dpsi[1][i]; // dphi_i/dx
    //     _dphi[i](1) = Jinv[1][0]*dpsi[0][i] + Jinv[1][1]*dpsi[1][i]; // dphi_i/dy
    // }

    _JxW = qp.second*jac;

    this->_normal(0) = -J[0][1];
    this->_normal(1) =  J[0][0];
    this->_normal.unit();

}

void BoundaryFEMFunction::TRI3FaceFunction(Element& elem, QGaussData qp)
{
    
    double dpsi[2][3];
    double J[2][3]    = {{0.0, 0.0, 0.0}, {0.0, 0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double x[3], y[3], z[3];
   
    _phi.resize(3);
    _dphi.resize(3);
    /*
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

    double detJ = 0.0;

    for (int i=0; i < 3; i++)
    {
        double tmp = (J[0][cyc3[i+2]]*J[1][cyc3[i+1]]-J[1][cyc3[i+2]]*J[0][cyc3[i+1]]);
        detJ+=tmp*tmp;
    }
              
    detJ=sqrt(detJ)

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
    */

}

void BoundaryFEMFunction::QUAD4FaceFunction(Element& elem, QGaussData qp)
{
    /*
    double dpsi[2][4];
    double J[2][2]    = {{0.0, 0.0}, {0.0, 0.0}};
    double Jinv[2][2] = {{0.0, 0.0}, {0.0, 0.0}};
    double x[3], y[3];
    
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
    */
   
}
