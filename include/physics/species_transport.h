#ifndef SPECIES_TRANSPORT_H
#define SPECIES_TRANSPORT_H

// #include <vector>

// /**
//  * @brief 
//  * 
//  *  Implements the convection diffusion reaction equation for species transport.
//  *  The equation is given by:
//  *      \f[
//  *         \frac{\partial c}{\partial t} + \nabla \cdot (\vec{v} c) - \nabla \cdot (\Gamma \nabla c) \sigma c = S
//  *     \f]
//  *      where:
//  *         - c is the concentration of the species
//  *         - \vec{v} is the velocity field
//  *         - \Gamma is the diffusion coefficient
//  *         - \sigma is the reaction coefficient   
//  *         - S is the source term
//  * 
//  */

// #include "transient_implicit_system.h"

// class SpeciesTransport {

//     public:
//         SpeciesTransport(unsigned int nspecies);
//         ~SpeciesTransport();

//         void set_diffusion_coefficient(unsigned int species, double value);
//         void set_reaction_coefficient(unsigned int species, double value);
//         void set_time_step(double dt);
        


//     private:
//         unsigned int nspecies;
//         std::vector<double> diffusion_coefficient;
//         std::vector<double> reaction_coefficient;
//         std::unique_ptr<TransientImplicitSystem> system;

// }



#endif /* SPECIES_TRANSPORT_H */
