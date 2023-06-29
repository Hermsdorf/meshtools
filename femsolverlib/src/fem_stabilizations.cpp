
#include <math.h>

#include "fem_stabilizations.h"

double CAUStab(double u, double u_old, Gradient grad_u, double f,
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

double TAUStab(RealVector velocity, RealTensor G, double k, double dt_stab, double dt)
{
    double tmp = (velocity) * (G.mult(velocity)) + (k * k) * (G.contract(G)) + dt_stab*4.0/(dt*dt);
    double tau = 1.0/sqrt(tmp);

    return tau;
}