#ifndef FEM_STABILIZATIONS_H__
#define FEM_STABILIZATIONS_H__

#include "numeric_vector.h"
#include "tensor.h"
#include "fem_functions.h"

class FemStabilization {
    public:
    
    FemStabilization(std::unique_ptr<FEMFunction>& fem_function) : 
           _fem_function(fem_function), delta_factor(0.0), phi_reference(1.0), beta(1.0) {}

    double supg_stabilization(RealVector velocity, double diffusivity, double dt=1.0);

    double yzb_stabilization(double pde_residuo, RealVector grad_u);

    void set_time_step_factor(double delta_factor) { this->delta_factor = delta_factor; }

    void set_yzb_phi_reference(double phi_reference)  { this->phi_reference = phi_reference; }

    void set_yzb_beta(double beta) { this->beta = beta; }

    void set_yzb_fopc(double fopc) { this->fopc = fopc; }

    private:
        // Store the FEMFunction reference
        std::unique_ptr<FEMFunction>& _fem_function;
        double delta_factor = 1.0;

        // yzb stabilization parameter
        double beta;

        double phi_reference;

        double fopc = 1.0;
        

};


// CAU stabilization parameter
double CAUStab(double u, double u_old, Gradient grad_u, double f,
               RealVector velocity, double sigma, double K,
               double dt, double h_caract);

// SUPG stabilization parameter
double TAUStab(RealVector velocity, RealTensor G, double k, double dt_stab=1.0,
               double dt=1.0);

#endif /* FEM_STABILIZATIONS_H__ */
