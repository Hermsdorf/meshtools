
#include <iostream>
#include "parallel_mesh.h"
#include "numeric_vector.h"
#include "tensor.h"

#include "fem_functions.h"

FEMFunction::FEMFunction() { }

double FEMFunction::CAUStab(double u, double u_old, Gradient grad_u, double f,
                RealVector velocity, double sigma, double K, double dt, double h_caract)
{
    // double res = velocity*grad_u - sigma*u - f;

    // double velocity_norm = velocity.norm();
    // double dphi_norm     = std::max(1.0E-10, grad_u.norm());
    
    // RealVector v;
    // if(dphi_norm == 1.0E-10)
    //     v = velocity;
    // else
    //     v = velocity - (grad_u*res)/(dphi_norm*dphi_norm);

    // RealVector be;
    // for(int i = 0 ; i < 3 ; i++)
    //     be(i) = velocity(i)*(dxi(i) + deta(i) + dzeta(i));
    
    // double be_norm = be.norm();
    // double he = 2.0*velocity_norm/be_norm;
    // double Pe = he*velocity_norm/(2*K);
    // double tau_e = std::max(1.0E-10, 1.0 - (1.0/Pe));


    // RealVector be_c;
    // for(int i = 0 ; i < 3 ; i++){
    //     double velocity_diff = velocity(i) - v(i);
    //     be_c(i) = velocity_diff*(dxi(i) + deta(i) + dzeta(i));
    // }

    // double be_c_norm = be_c.norm();
    // RealVector diff_v = velocity - v;
    // double     diff_v_norm = diff_v.norm();

    // be_c_norm = std::max(1.0E-10,be_c_norm);
    // double he_c = 2*diff_v_norm/be_c_norm;
    // double Pe_c = (he_c*diff_v_norm)/(2.0*std::abs(K));
    // double tau_c = std::max(0.0, 1.0 - (1.0/Pe_c));

    // double res_vel_dphi = std::abs(res)/(velocity_norm*dphi_norm);
    // double tauc_hc_tau_h = (tau_c*he_c)/(tau_e*he);
    
    // if(res_vel_dphi >= tauc_hc_tau_h)
    //     return 0.0;
    // else
    //     return (tau_e*he*0.5)*(tauc_hc_tau_h - res_vel_dphi)*(std::abs(res)/dphi_norm);


    double res_mass = (u - u_old)/dt;
    double res_adv  = (velocity * grad_u);
    double residuo  = res_mass + res_adv - sigma*u - f;
    double gcnorm = grad_u.norm();
    gcnorm = std::max(1.0E-10, gcnorm);
    double ogcnorm = 1.0 / gcnorm;
    double aux3 = res_adv / (ogcnorm * ogcnorm);
    RealVector b(grad_u(0) * aux3, grad_u(1) * aux3);
    double bnorm = b.norm();
    bnorm = std::max(bnorm, 1.0E-10);
    double bdb = K * bnorm*bnorm;
    bdb = std::max(bdb, 1.0E-10);
    double Pe_p = 0.5 * h_caract * (bnorm * bnorm * bnorm) / bdb;
    double alpha_c = std::min(0.25 * Pe_p, 1.0);
    
    double cau = 0.5 * h_caract * alpha_c * residuo * ogcnorm; 
    
    return cau;
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

    _dxi(0) = J[0][0];
    _dxi(1) = J[0][1];
    _deta(0)  = J[1][0];
    _deta(1)  = J[1][1];
}
