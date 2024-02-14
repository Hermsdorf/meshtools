
#include <set>
#include "meshtools.h"
#include "transient_implicit_system.h"

/**
 * Constructor
 * 
 * @param mesh: parallel mesh object
 * @param name: name of the system
*/
TransientImplicitSystem::TransientImplicitSystem(ParallelMesh &mesh, std::string name):
    NonLinearImplicitSystem(mesh, name)
    {
        _t  = 0.0;
        _dt = 0.0;
        _n_write = 0;
        _init_function = nullptr;
    }

/**
 * Initializes the system filling the solution vector with zeros
 * and duplicating to create the old and older solution vectors.
 * Furthermore, it applies the initial conditions and initializes
 * the time and timestep variables.
*/
void TransientImplicitSystem::init()
{
    NonLinearImplicitSystem::init();
    VecDuplicate(_solution_local, &_old_solution_local);
    VecDuplicate(_solution_local, &_older_solution_local);

    apply_initial_conditions();
    _t       = 0.0;
    timestep = 0;
}

/**
 * Returns the solution vector at t-dt (older solution)
 * 
 * @return: double* solution array with the older solution
*/
double* TransientImplicitSystem::get_old_solution_array()
{
    double* solution_array;
    VecGetArray(this->_old_solution_local, &solution_array);
    return solution_array;
}

/**
 * Returns the solution vector at t-2*dt (older solution)
 * 
 * @return: double* solution array with the older solution
*/
double* TransientImplicitSystem::get_older_solution_array()
{
    double* solution_array;
    VecGetArray(this->_older_solution_local, &solution_array);
    return solution_array;
}

/**
 * Restores the solution array
 * 
 * @param solution_array: double* solution array
*/
void TransientImplicitSystem::restore_old_solution_array(double** solution_array)
{
    VecRestoreArray(this->_old_solution_local, solution_array);
}

/**
 * Restores the solution array
 * 
 * @param solution_array: double* solution array
*/
void TransientImplicitSystem::restore_older_solution_array(double** solution_array)
{
    VecRestoreArray(this->_older_solution_local, solution_array);
}

/**
 * Solves the time step. For do this it solves a nonlinear system
 * substituting the incognit values by the previous solution values.
 * When the difference between the previous solution and the current
 * solution is less than the tolerance, the method stops.
 * 
 * For a linear system, the method breaks in the second iteration with
 * the solution.
*/
void TransientImplicitSystem::solve_time_step()
{
    _t += _dt;

    PetscPrintf(MeshTools::Comm(),"Solving time %0.4f\n", _t);
    VecCopy(_old_solution_local, _older_solution_local);
    VecCopy(_solution_local, _old_solution_local);
    
    // getting solution at t+dt
    this->solve_nonlinear_system();

    MatZeroEntries(this->_A);
    VecZeroEntries(this->_rhs);

    timestep++;
    update_deltat();
}

/**
 * Solves the nonlinear system. For do this it solves a linear system
 * substituting the incognit values by the previous solution values.
 * When the difference between the previous solution and the current
 * solution is less than the tolerance, the method stops.
*/
void TransientImplicitSystem::solve_nonlinear_system()
{
    unsigned int iter = 0;
    double _solution_norm;

    unsigned int _max_nonlinear_iterations = get_nonlinear_max_iter();
    float _tolerance                       = get_nonlinear_tolerance();

    double initial_tol = _linear_tolerance;
    double final_linear_residual;

    KSPSetTolerances(this->_ksp, initial_tol, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT);

    while(iter < _max_nonlinear_iterations)
    {
        VecCopy(this->_solution, this->_previous_nonlinear_solution);

        // Calls the method to assemble the system
        this->_assemble_function(this);
        this->solve_linear_system();

        KSPGetResidualNorm(this->_ksp, &final_linear_residual);
        if((iter == 0) && final_linear_residual >= _tolerance)
        {
            initial_tol*=1.0E-3;
            PetscPrintf(MeshTools::Comm(), "   Initial Linear solution rejected!\n   Pick an even lower linear solver tolerance\n   and try again\n");
            set_linear_tolerance(initial_tol);
            this->solve_linear_system();
        }

        // Scales the solution vector by -1.0 and
        // adds the previous solution vector
        VecAXPY(this->_previous_nonlinear_solution,-1.0, this->_solution);

        // Takes the euclidian norm of previous_solution
        // vector and stores it in _solution_norm
        VecNorm(this->_previous_nonlinear_solution, NORM_2, &_solution_norm);


        if(_solution_norm < _tolerance)
            break;

        iter++;

        MatZeroEntries(this->_A);
        VecZeroEntries(this->_rhs);

        // For the inexact Newton
        // method, the linear solver tolerance needs to decrease as we get closer to
        // the solution to ensure quadratic convergence.  The new linear solver tolerance
        // is chosen (heuristically) as the square of the previous linear system residual norm.
        // Real flr2 = final_linear_residual*final_linear_residual;

        double flr2 = final_linear_residual*final_linear_residual;
        double new_linear_solver_tolerance = std::min(std::max(flr2,10E-16), initial_tol);

        KSPSetTolerances(this->_ksp, new_linear_solver_tolerance, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT);
    }

    MatZeroEntries(this->_A);
    VecZeroEntries(this->_rhs);

    if(_max_nonlinear_iterations > 1){
        PetscPrintf(MeshTools::Comm(), "Nonlinear number of iterations = %d\n", iter+1);
        PetscPrintf(MeshTools::Comm(), "Nonlinear final norm of residual: %8.8e\n", _solution_norm);
    }
}

void TransientImplicitSystem::update_deltat()
{
    //TODO: Implement timestep control based on CFL condition
}


/**
 * Adds an initial condition to the system by pushing back
 * the initial condition to the vector of initial conditions.
 * 
 * @param ic: InitialCondition object
*/
void TransientImplicitSystem::add_initial_condition(InitialCondition ic)
{
    _initial_conditions.push_back(ic);
}


/**
 * Applies the initial conditions to the solution vector.
 * 
 * If a init function is defined it is called guessing the
 * user already applied the initial conditions there. Otherwise
 * the initial conditions are applied by looping over the mesh
 * elements and nodes.
*/
void TransientImplicitSystem::apply_initial_conditions()
{

    if(this->_init_function)
    {
        this->_init_function(this);
        return;
    }

    std::vector< std::set<unsigned int> > nodelist(_initial_conditions.size());
    
    for(int iel=0; iel < this->_mesh.get_n_elements(); iel++)
    {
        auto *conn  = this->_mesh.getElementConn(iel);
        auto  connsz = _mesh.getElementConnSize(iel);
        for(int i=0; i < _initial_conditions.size(); i++)
        {
            if(_initial_conditions[i].get_region_id() == _mesh.getElementTag(iel))
            {
                for(int j=0; j < connsz; j++)
                {
                    nodelist[i].insert(conn[j]);
                }
            }
        }
    }

    auto coords = _mesh.getCoord();
    auto *solution = this->get_local_solution_array();

    for(int i=0; i < _initial_conditions.size(); i++)
    {
        auto dof_id    = _initial_conditions[i].get_dof_id();
        auto region_id = _initial_conditions[i].get_region_id();
        for(auto it    = nodelist[i].begin(); it != nodelist[i].end(); it++)
        {
            int  node_id = *it;
            double     x = coords[node_id*3];
            double     y = coords[node_id*3+1];
            double     z = coords[node_id*3+2];
            double     value = _initial_conditions[i].get_value(x,y,z,0);
            solution[node_id*_n_dof + dof_id] = value;
        }
    }
    this->restore_local_solution_array(&solution);
}

/**
 * Attaches the assemble function implemented by the user
 * to the system that will be called
 * 
 * @param _assemble: function pointer to the assemble function
*/
void TransientImplicitSystem::attach_assemble(void _assemble(TransientImplicitSystem*))
{
    _assemble_function = _assemble;
}

/**
 * Attaches the init function implemented by the user
 * to the system that will be called
 * 
 * @param _init: function pointer to the init function
*/
void TransientImplicitSystem::attach_init_function(void _init(TransientImplicitSystem*))
{
   _init_function = _init;
}

/**
 * Write the solution to a file
 * 
 * @param filename: string with the filename
*/
void TransientImplicitSystem::write_result(string filename)
{
    auto n_nodes = _mesh.get_n_nodes();
    
    std::vector<double> solution(n_nodes*_n_dof);
    unsigned int offset  = 0;
    double *solution_ptr = get_local_solution_array();
    MeshIODataAppended info;
    for(int i = 0; i < _n_dof; i++)
    {
        for(int ino = 0; ino < n_nodes; ino++)
            solution[ino+offset] = solution_ptr[ino*_n_dof + i];

        std::string var = this->_variables_names[0];
        info.addPointDataInfo(var.c_str(), Float64, &solution[offset]);
        offset += n_nodes;
    }
    info.addTimeDataInfo(this->get_time(), _n_write++);
    
    this->_mesh.writePVTK(filename.c_str(), &info);

    restore_local_solution_array(&solution_ptr);
}
