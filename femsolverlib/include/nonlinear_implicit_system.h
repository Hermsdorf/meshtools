#ifndef NONLINEAR_IMPLICIT_SYSTEM_H__
#define NONLINEAR_IMPLICIT_SYSTEM_H__

// TODO:
// 1. Implementar NonLinearImplicitSystem::solve_nonlinear_system()
// 2. Implementar NonLinearImplicitSystem::set_nonlinear_tolerance()
// 3. Implementar NonLinearImplicitSystem::set_max_nonlinear_iterations()
class NonLinearImplicitSystem : public ImplicitSystem
{
    public:
        NonLinearImplicitSystem(ParallelMesh &mesh, std::string name);
        void   attach_assemble(void assemble_system(NonLinearImplicitSystem*)); 
        void   solve();
        void   set_nonlinear_tolerance(double tol);
        ~NonLinearImplicitSystem();

    protected:
        
        void   solve_nonlinear_system();
        /*
           copiar solução atual para _prev_solution
           k = 0;
           while(k < _max_nonlinear_iterations)
           {
                assemble_system(this);
                solve();
                if (norma(_solution - _prev_solution) < tol)
                    break;
                
                copiar _solution para _prev_solution
           }
        
        */

        Vec          _prev_solution;
        double       _nonlinear_tolerance;
        unsigned int _max_nonlinear_iterations;

    private:
        void (* _assemble_system)(NonLinearImplicitSystem*  _system);
       

};


#endif /* NONLINEAR_IMPLICIT_SYSTEM_H__ */
