
#include <set>
#include "meshtools.h"
#include "transient_implicit_system.h"

TransientImplicitSystem::TransientImplicitSystem(ParallelMesh &mesh, std::string name):
    ImplicitSystem(mesh, name)
    {
        _t  = 0.0;
        _dt = 0.0;
        _n_write = 0;
        _init_function = nullptr;
    }

void TransientImplicitSystem::init()
{
    
    ImplicitSystem::init();
    VecDuplicate(_solution_local, &_old_solution_local);
    VecDuplicate(_solution_local, &_older_solution_local);

    apply_initial_conditions();
    _t       = 0.0;
    timestep = 0;
}

double* TransientImplicitSystem::get_old_solution_array()
{
    double* solution_array;
    VecGetArray(this->_old_solution_local, &solution_array);
    return solution_array;
}

double* TransientImplicitSystem::get_older_solution_array()
{
    double* solution_array;
    VecGetArray(this->_older_solution_local, &solution_array);
    return solution_array;
}

void TransientImplicitSystem::restore_old_solution_array(double** solution_array)
{
    VecRestoreArray(this->_old_solution_local, solution_array);
}

void TransientImplicitSystem::restore_older_solution_array(double** solution_array)
{
    VecRestoreArray(this->_older_solution_local, solution_array);
}

void TransientImplicitSystem::solve_time_step()
{
    _t += _dt;

    PetscPrintf(MeshTools::Comm(),"Solving time %0.4f\n", _t);
    VecCopy(_old_solution_local, _older_solution_local);
    VecCopy(_solution_local, _old_solution_local);
    
 


    this->_assemble_function(this);

    // getting solution at t+dt
    ImplicitSystem::solve_linear_system();

    MatZeroEntries(this->_A);
    VecZeroEntries(this->_rhs);
    
    timestep++;
    update_deltat();
}

void TransientImplicitSystem::update_deltat()
{
    //TODO: Implement timestep control based on CFL condition
}

void TransientImplicitSystem::add_initial_condition(InitialCondition ic)
{
    _initial_conditions.push_back(ic);
}


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


void TransientImplicitSystem::attach_assemble(void _assemble(TransientImplicitSystem*))
{
    _assemble_function = _assemble;
}

 void TransientImplicitSystem::attach_init_function(void _init(TransientImplicitSystem*))
 {
    _init_function = _init;
 }

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