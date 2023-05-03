
#include "boundary_fem_functions.h"

unsigned int cyc3[5]={0, 1, 2, 0, 1 };

BoundaryFEMFunction::BoundaryFEMFunction()
{


}

void BoundaryFEMFunction::ComputeFunction(SurfaceElement& elem, QGaussData qp)
{   
    this->_normal = elem.calculate_normal();

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

void BoundaryFEMFunction::EDGEFaceFunction(SurfaceElement& elem, QGaussData qp)
{
    _phi.resize(2);
    _dphi.resize(2);


    std::vector<RealVector> dpsi(2);
    RealVector dxyzdxi;


    double xi = qp.first(0);

    // EDGE2 shape functions
    _phi[0] = 0.5*(1-xi); // N1
    _phi[1] = 0.5*(1+xi); // N2

    // EDGE2 shape functions derivatives
    dpsi[0](0) = -0.5;  //dN1/dxi
    dpsi[1](0) =  0.5;  //dN2/dxi

    // compute x, dxdxi at the quadrature points

    _xyz.zero();
    dxyzdxi.zero();
    for(int i=0; i<elem.n_nodes(); i++)
    {
        _xyz.add_scaled(phi[i], elem.node(i));
        dxyzdxi.add_scaled(dpsi[i](0), elem.node(i));
    }

    // compute the determinant of the Jacobian
    double jac = dxyzdxi.norm();

    // compute the inverse of the Jacobian
    double invjac = 1.0/jac;

    _JxW = qp.second*jac;

    this->_normal(0) = -dxyzdxi(1);
    this->_normal(1) =  dxyzdxi(0);
    this->_normal.unit();

    this->_tangents.resize(1);
    this->_tangents[0] = dxyzdxi.unit();

}

void BoundaryFEMFunction::TRI3FaceFunction(SurfaceElement& elem, QGaussData qp)
{
    
    std::vector<RealVector> dpsi(3);

    _phi.resize(3);
    _dphi.resize(3);

    RealVector dxyzdxi;
    RealVector dxyzdeta;

    double xi  = qp.first(0);
    double eta = qp.first(1);

    // shape function
    _phi[0] = 1.0 - xi - eta; // N1
    _phi[1] = xi;             // N2
    _phi[2] = eta;            // N3

    // shape function derivative 
    dpsi[0](0) = -1.0; // dN1/dxi
    dpsi[0](1) = -1.0; // dN1/deta
    dpsi[1](0) =  1.0; // dN2/dxi
    dpsi[1](1) =  0.0; // dN2/deta
    dpsi[2](0) =  0.0; // dN3/dxi
    dpsi[2](1) =  1.0; // dN3/deta


    _xyz.zero();
    dxyzdxi.zero();
    dxyzdeta.zero();
    for(int i=0; i<elem.n_nodes(); i++)
    {
        _xyz.add_scaled(phi[i], elem.node(i));
        dxyzdxi.add_scaled(dpsi[i](0), elem.node(i));
        dxyzdeta.add_scaled(dpsi[i](1), elem.node(i));
    }

    const double g11 = dxyzdxi(0)*dxyzdxi(0)  + dxyzdxi(1)*dxyzdxi(1)                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              + dxyzdxi(2)*dxyzdxi(2);
    const double g12 = dxyzdxi(0)*dxyzdeta(0) + dxyzdxi(1)*dxyzdeta(1)   + dxyzdxi(2)*dxyzdeta(2);
    const double g21 = g12;
    const double g22 = dxyzdeta(0)*dxyzdeta(0) + dxyzdeta(1)*dxyzdeta(1) + dxyzdeta(2)*dxyzdeta(2);

    const double detJ = std::sqrt(g11*g22 - g12*g21);

    if(detJ < 0.0)
    {
        std::cout << "Error: detJ < 0.0  -- det = " << detJ << std::endl;
        exit(1);
    }

    _JxW = qp.second*detJ;

    const Point n = dxyzdxi.cross(dxyzdeta);
    this->_normal = n.unit();
    this->_tangents.resize(2);
    this->_tangents[0] = dxyzdxi.unit();
    this->_tangents[1] = n.cross(dxyzdxi).unit();

}

void BoundaryFEMFunction::QUAD4FaceFunction(SurfaceElement& elem, QGaussData qp)
{
    std::vector<RealVector> dpsi(4);
    RealVector dxyzdxi;
    RealVector dxyzdeta;

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
    dpsi[0](0) = -0.25*(1.0-eta);  // dN1/dxi
    dpsi[1](0) =  0.25*(1.0-eta);  // dN2/dxi
    dpsi[2](0) =  0.25*(1.0+eta);  // dN3/dxi
    dpsi[3](0) = -0.25*(1.0+eta);  // dN4/dxi

    dpsi[0](1) = -0.25*(1.0-xi);  // dN1/deta
    dpsi[1](1) = -0.25*(1.0+xi);  // dN2/deta
    dpsi[2](1) =  0.25*(1.0+xi);  // dN3/deta
    dpsi[3](1) =  0.25*(1.0-xi);  // dN4/deta

    _xyz.zero();
    dxyzdxi.zero();
    dxyzdeta.zero();
    for(int i=0; i<elem.n_nodes(); i++)
    {
        _xyz.add_scaled(phi[i], elem.node(i));
        dxyzdxi.add_scaled(dpsi[i](0), elem.node(i));
        dxyzdeta.add_scaled(dpsi[i](1), elem.node(i));
    }

    const double g11 = dxyzdxi(0)*dxyzdxi(0) + dxyzdxi(1)*dxyzdxi(1) + dxyzdxi(2)*dxyzdxi(2);
    const double g12 = dxyzdxi(0)*dxyzdeta(0) + dxyzdxi(1)*dxyzdeta(1) + dxyzdxi(2)*dxyzdeta(2);
    const double g21 = g12;
    const double g22 = dxyzdeta(0)*dxyzdeta(0) + dxyzdeta(1)*dxyzdeta(1) + dxyzdeta(2)*dxyzdeta(2);

    const double detJ = std::sqrt(g11*g22 - g12*g21);

    if(detJ < 0.0)
    {
        std::cout << "Error: detJ < 0.0  -- det = " << detJ << std::endl;
        exit(1);
    }

    _JxW = qp.second*detJ;

    const Point n = dxyzdxi.cross(dxyzdeta);
    this->_normal = n.unit();
    this->_tangents.resize(2);
    this->_tangents[0] = dxyzdxi.unit();
    this->_tangents[1] = n.cross(dxyzdxi).unit();
   
}
