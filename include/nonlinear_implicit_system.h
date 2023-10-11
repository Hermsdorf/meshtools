#ifndef NONLINEAR_IMPLICIT_SYSTEM_H
#define NONLINEAR_IMPLICIT_SYSTEM_H

#include "implicit_system.h"

class NonLinearImplicitSystem : public ImplicitSystem
{
    public:
        NonLinearImplicitSystem(ParallelMesh &mesh, std::string name);

        void   attach_assemble(void _assemble(NonLinearImplicitSystem*));
        void   solve();
        void   init();
        void   set_nonlinear_tolerance(double tolerance) { _tolerance = tolerance; };
        void   set_nonlinear_max_iter(unsigned int max_iter) { _max_nonlinear_iterations = max_iter; };
        double get_nonlinear_tolerance() { return _tolerance; };
        unsigned int get_nonlinear_max_iter() { return _max_nonlinear_iterations; };
        Vec&    get_previous_nonlinear_solution() { return _previous_nonlinear_solution; };
        void    set_previous_nonlinear_solution(Vec& previous_solution) { _previous_nonlinear_solution = _previous_nonlinear_solution; };
        
        ~NonLinearImplicitSystem();
    
    protected:
        void solve_nonlinear_system();
        Vec          _previous_nonlinear_solution;

    private:

        double       _tolerance;
        unsigned int _max_nonlinear_iterations;
        
        void (* _assemble_function)(NonLinearImplicitSystem*  _system);

};

#endif // NONLINEAR_IMPLICIT_SYSTEM_H