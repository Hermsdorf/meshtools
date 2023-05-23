
#include "nonlinear_implicit_system.h"

NonLinearImplicitSystem::NonLinearImplicitSystem(ParallelMesh &mesh, std::string name): ImplicitSystem(mesh, name)
{
    _tolerance = 1e-6;
    _max_nonlinear_iterarions = 1e6;
}

NonLinearImplicitSystem::~NonLinearImplicitSystem()
{
    KSPDestroy(&this->_ksp);
    VecDestroy(&this->_solution);
    VecDestroy(&this->_previous_solution);
    VecDestroy(&this->_rhs);
    MatDestroy(&this->_A);
    VecDestroy(&this->_solution_local);
    VecScatterDestroy(&this->_scatter);
}

void NonLinearImplicitSystem::attach_assemble(void _assemble(NonLinearImplicitSystem*) )
{
    _assemble_function = _assemble;
}

void NonLinearImplicitSystem::solve_nonlinear_system()
{
    unsigned int iter = 0;
    while(iter < _max_nonlinear_iterarions)
    {
        _assemble_function(this);

        MatAssemblyBegin(_A, MAT_FINAL_ASSEMBLY);
        MatAssemblyEnd(_A, MAT_FINAL_ASSEMBLY);

        VecAssemblyBegin(this->_rhs);
        VecAssemblyEnd(this->_rhs);

        this->apply_dirichlet_boundary_conditions();

        KSPSetUp(this->_ksp);
        KSPSolve(this->_ksp, this->_rhs, this->_solution);

        if(_solution - _previous_solution <= _tolerance)
            break;
        
        _previous_solution = _solution;
        iter++;
    }
}

void NonLinearImplicitSystem::solve()
{
    _assemble_function(this);
    solve_nonlinear_system();
}