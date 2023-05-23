#ifndef NONLINEAR_IMPLICIT_SYSTEM_H
#define NONLINEAR_IMPLICIT_SYSTEM_H

#include "implicit_system.h"

class NonLinearImplicitSystem : public ImplicitSystem
{
    public:
        NonLinearImplicitSystem(ParallelMesh &mesh, std::string name);

        void   attach_assemble(void _assemble(NonLinearImplicitSystem*));
        void   solve();
        void   set_nonlinear_tolerance(double tolerance) { _tolerance = tolerance; };
        void   set_nonlinear_max_iter(unsigned int max_iter) { _max_nonlinear_iterarions = max_iter; };
        
        ~NonLinearImplicitSystem();
    
    protected:
        void solve_nonlinear_system();

    private:
        Vec          _previous_solution;
        double       _tolerance;
        unsigned int _max_nonlinear_iterarions;
        
        void (* _assemble_function)(NonLinearImplicitSystem*  _system);

};

#endif // NONLINEAR_IMPLICIT_SYSTEM_H