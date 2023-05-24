
#include "nonlinear_implicit_system.h"

NonLinearImplicitSystem::NonLinearImplicitSystem(ParallelMesh &mesh, std::string name): ImplicitSystem(mesh, name)
{
    _tolerance = 1e-6;
    _max_nonlinear_iterarions = 20;
}

void NonLinearImplicitSystem::init()
{
    ImplicitSystem::init();
    VecDuplicate(this->_solution, &this->_previous_solution);
}



NonLinearImplicitSystem::~NonLinearImplicitSystem()
{
    VecDestroy(&this->_previous_solution);
    ImplicitSystem::~ImplicitSystem();
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
        VecCopy(this->_solution, this->_previous_solution);

        
        this->_assemble_function(this);
        this->solve_linear_system();

        double _solution_norm;
        VecAXPY(this->_previous_solution,-1.0, this->_solution);
        VecNorm(this->_previous_solution, NORM_2, &_solution_norm);

        if(_solution_norm < _tolerance)
            break;
        
       
        iter++;

        MatZeroEntries(this->_A);
        VecZeroEntries(this->_rhs);
    }
}

void NonLinearImplicitSystem::solve()
{
    solve_nonlinear_system();
}