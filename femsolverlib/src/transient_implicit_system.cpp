
#include <set>
#include "transient_implicit_system.h"

TransientImplicitSystem::TransientImplicitSystem(ParallelMesh &mesh, std::string name):
    ImplicitSystem(mesh, name)
    {

    }

void TransientImplicitSystem::init()
{
    
    ImplicitSystem::init();
    apply_initial_conditions();
    VecDuplicate(_solution_local, &_old_solution_local);
    VecDuplicate(_solution_local, &_older_solution_local);
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

void TransientImplicitSystem::solve()
{
    ImplicitSystem::solve();
    VecCopy(_old_solution_local, _older_solution_local);
    VecCopy(_solution_local, _old_solution_local);
    timestep++;
}

void TransientImplicitSystem::add_initial_condition(InitialCondition ic)
{
    _initial_conditions.push_back(ic);
}


void TransientImplicitSystem::apply_initial_conditions()
{

    std::vector< std::set<unsigned int> > nodelist(_initial_conditions.size());
    auto region_ids = _mesh.getPhysicalTag();
    for(int iel=0; iel < this->_mesh.get_n_elements(); iel++)
    {
        auto *conn  = this->_mesh.getElementConn(iel);
        auto  connsz = _mesh.getElementConnSize(iel);
        for(int i=0; i < _initial_conditions.size(); i++)
        {
            if(_initial_conditions[i].get_region_id() == region_ids[iel])
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
            double     x = coords[*it*3];
            double     y = coords[*it*3+1];
            double     z = coords[*it*3+2];
            double     value = _initial_conditions[i].get_value(x,y,z,0);
            solution[*it*_n_dof + dof_id] = value;
        }
    }
    this->restore_local_solution_array(&solution);
}


