
#include <set>
#include <algorithm>
#include <cassert>
#include "meshtools.h"
#include "equation_manager.h"
using namespace std;

EquationManager::EquationManager(std::unique_ptr<ParallelMesh> &mesh):
    _mesh(mesh),
    _ndof(0),
    _first_global_equation_index(0),
    _prepared_to_use(false)
    {

    }

EquationManager::~EquationManager()
{
    _equation_indices.clear();
    _boundaries.clear();
}

unsigned int EquationManager::first_global_equation_index()
{
    return this->_first_global_equation_index;
}

unsigned int EquationManager::n_local_equations()
{
    return this->_n_local_equations;
}

void EquationManager::global_indices(int id_dof, std::vector<unsigned int>& conn, std::vector<int>& global_equation)
{

    global_equation.resize(conn.size());
    for(int i = 0 ; i < conn.size() ; i++)
    {
        unsigned int node_id = conn[i];
        for(int j = 0 ; j < _ndof; j++)
        {
            global_equation[i*_ndof+j] = _equation_indices[node_id*_ndof + id_dof];
        }
    }

}


void EquationManager::local_indices(int id_dof, std::vector<unsigned int>& conn, std::vector<int>& local_equation)
{

    local_equation.resize(conn.size());
    for(int i = 0 ; i < conn.size() ; i++)
    {
        unsigned int node_id = conn[i];
        for(int j = 0 ; j < _ndof; j++)
        {
            local_equation[i*_ndof+j] = node_id*_ndof + id_dof;
        }
    }

}

void EquationManager::equation_indices(int id_dof, std::vector<unsigned int> &conn, std::vector<unsigned int> & global_equation)
{
    std::vector<int> dof_required;

    // When dof id is -1 it means that we want to get the equation for every dof in the system
    if(id_dof == -1)
    {
        dof_required.resize(_ndof);
        for(int i = 0 ; i < _ndof ; i++)
            dof_required[i] = i;
    }   
    else
    {
        dof_required.resize(1);
        dof_required[0] = id_dof;
    }
    
    for(int i = 0 ; i < conn.size() ; i++)
    {
        unsigned int node_id = conn[i];
        
        for(int j = 0 ; j < dof_required.size(); j++)
        {
            unsigned int dof_id = dof_required[j];
            global_equation[i*dof_required.size() + j] = _equation_indices[node_id*_ndof + dof_id];
        }
    }

    dof_required.clear();
}   

void EquationManager::add_dirichlet_boundary(DirichletBoundary &boundary)
{
    for(auto it = _boundaries.begin(); it != _boundaries.end(); ++it)
        if(*it == boundary) return;

    _boundaries.push_back(boundary);
}

void EquationManager::prepare_to_use()
{

    //* 1. Defining nodes with boundary conditions
    int n_nodes                           = _mesh->get_n_nodes();
    int n_boundary_elements               = _mesh->get_n_surface_elements();
    std::vector<unsigned int> &  node_ids = _mesh->get_node_index_vector();
    std::vector<int> &tags                = _mesh->get_element_physical_tag_vector();

    _equation_indices.resize(n_nodes*_ndof);
    _boundary_nodes_map.resize(_boundaries.size());

    int bnd_id = 0;
    for(auto it = _boundaries.begin(); it != _boundaries.end(); ++it)
    {
        // Get each node from its `it` boundary 
        int boundary_id = it->get_boundary_id(); // Gmsh id
        int dof_id      = it->get_dof_id();      // Meshtools id

        
        std::set<int>  boundary_nodes;

        for(int iel = 0; iel < n_boundary_elements; iel++)
        {
            if(tags[iel] == boundary_id)
            {
                std::vector<unsigned int> conn;
                _mesh->get_surface_element_connectivity(iel, conn);
                for(int ino = 0; ino < conn.size(); ++ino)
                    boundary_nodes.insert(conn[ino]);
            }
        }

       
        _boundary_nodes_map[bnd_id].resize(boundary_nodes.size());

        unsigned int ibcno = 0;
        for(auto bnd_node_iter =  boundary_nodes.begin(); bnd_node_iter != boundary_nodes.end(); ++bnd_node_iter)
        {
                int node_id = *bnd_node_iter;
                // flag indicanting that there is no equation to this node because it is a boundary node (with its respective boundary condition)
                //_equation_indices[node_id*_ndof + dof_id] = -1;
                _boundary_nodes_map[bnd_id][ibcno++] = node_id;
        }
        bnd_id++;

    }


    for(int ino = 0; ino < n_nodes; ino++)
    {
        for(int idof = 0; idof < _ndof; idof++)
        {
            _equation_indices[ino*_ndof + idof] = node_ids[ino]*_ndof + idof;
        }
    }

    this->_first_global_equation_index =   _mesh->get_start_global_index()*_ndof;
    this->_n_local_equations           =   _mesh->get_n_local_nodes()*_ndof;
    
    _prepared_to_use = true;
    
}

void EquationManager::calculate_dnnz_onnz(std::vector<unsigned int> &dnnz, std::vector<unsigned int> &onnz)
{
    // Preallocation Matrix
    int n_nodes = _mesh->get_n_nodes();
    std::vector< std::set<int> > vdiag(_n_local_equations);
    std::vector< std::set<int> > voff(_n_local_equations);
    
    dnnz.resize(_n_local_equations);
    onnz.resize(_n_local_equations);

    int start = _first_global_equation_index;
    int end   = start + _n_local_equations;

    // Getting the d_nnz e o_nnz vector needed to matrix preallocation
    for (int iel = 0; iel < _mesh->get_n_elements(); ++iel)
    {
        std::vector<unsigned int> conn;
        _mesh->get_element_connectivity(iel, conn);
        int         connsz = conn.size();
        
        int n_equations = connsz*_ndof;

        std::vector<unsigned int> global_equations(n_equations);

        // stores global equation numbering for each node of the element in equations variable
        equation_indices(-1, conn, global_equations);

        for(int i = 0; i < n_equations; i++)
        {
            int eqI = global_equations[i];
            if(eqI >= 0)
            {
                if(eqI >= start && eqI < end)
                {
                    for(int j = 0; j < n_equations; j++)
                    {
                        int eqJ = global_equations[j];
                        if(eqJ >= start && eqJ < end)
                        {
                            vdiag[eqI-start].insert(eqJ);
                        }
                        else // it is a interface node that belongs to my master
                        {
                            voff[eqI-start].insert(eqJ);
                        }
                    }
                }
             }
        }
    }

    for(int i = 0; i < _n_local_equations; i++)
    {
        dnnz[i] = vdiag[i].size();
        onnz[i] = voff[i].size();
    }
}