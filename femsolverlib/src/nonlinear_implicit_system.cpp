
#include "nonlinear_implicit_system.h"

/**
 * Constructor
 * 
 * @param mesh: parallel mesh object
 * @param name: name of the system
*/
NonLinearImplicitSystem::NonLinearImplicitSystem(ParallelMesh &mesh, std::string name): ImplicitSystem(mesh, name)
{
    _tolerance = 1e-4;
    _max_nonlinear_iterarions = 20;
}

/**
 * Destructor
*/
NonLinearImplicitSystem::~NonLinearImplicitSystem()
{
    VecDestroy(&this->_previous_solution);
    ImplicitSystem::~ImplicitSystem();
}

/**
 * Initializes the system
*/
void NonLinearImplicitSystem::init()
{
    ImplicitSystem::init();
    VecDuplicate(this->_solution, &this->_previous_solution);
}


/**
 * It attaches the function that assembles the system.
 * This assemble function must be defined by the user
 * because it depends on the problem that is being solved.
 * 
 * @param _assemble: function that assembles the system
*/
void NonLinearImplicitSystem::attach_assemble(void _assemble(NonLinearImplicitSystem*) )
{
    _assemble_function = _assemble;
}

/**
 * Solves the nonlinear system. For do this it solves a linear system
 * substituting the incognit values by the previous solution values.
 * When the difference between the previous solution and the current
 * solution is less than the tolerance, the method stops.
*/
void NonLinearImplicitSystem::solve_nonlinear_system()
{
    unsigned int iter = 0;
    while(iter < _max_nonlinear_iterarions)
    {
        VecCopy(this->_solution, this->_previous_solution);

        // Calls the method to assemble the system
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