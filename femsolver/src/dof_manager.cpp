
#include <set>

#include "dof_manager.h"

 DofManager::DofManager(ParallelMesh &mesh):
    _mesh(mesh),
    _ndof(0),
    _first_global_dof_index(0),
    _last_global_dof_index(0),
    _prepared_to_use(false)
    {

    }

void DofManager::add_dirichlet_boundary(DirichletBoundary &boundary)
{
    for(auto it = this->_boundaries.begin(); it != this->_boundaries.end(); ++it)
        if(*it == boundary) return;

    this->_boundaries.push_back(boundary);
}


void DofManager::prepare_to_use()
{
    // 1. Definir os nos com condições de contorno

    int n_nodes             = _mesh.get_n_nodes();
    int n_boundary_elements = _mesh.get_n_face_elements();
    std::vector<int> &tags  = _mesh.getPhysicalTag();

    _dof_indices.resize(n_nodes*_ndof);

    for(auto it = this->_boundaries.begin(); it != this->_boundaries.end(); ++it)
    {
        // Extrai para cada contorno os nós 
        int boundary_id = it->get_boundary_id();
        int dof_id      = it->get_dof_id();

        std::set<int>  boundary_nodes;

        for(int iel = 0; iel < n_boundary_elements; iel++)
        {
            if(tags[iel] == boundary_id)
            {
                unsigned int connsize = _mesh.getSurfaceElementConnSize(iel);
                unsigned int *conn    = _mesh.getSurfaceElementConn(iel);
                for(int ino = 0; ino < connsize; ++ino)
                    boundary_nodes.insert(conn[ino]);

            }

        }

        int n_nodes_boundary = boundary_nodes.size();
        for(auto bnd_node_iter =  boundary_nodes.begin(); bnd_node_iter != boundary_nodes.end(); ++bns_node_iter)
        {
            int node_id = *bnd_node_iter;
            _dof_indices[node_id*_ndof + dof_id] = -1;
            
        }
        
    }

    // 2. Definir os nós com dof's
    int n_equations = 0;
    for(int i = 0; i < _dof_indices.size(); ++i)
    {
        if(_dof_indices[i] != -1)
        {
            _dof_indices[i] = n_equations;
            n_equations++;
        }
    }






}
    

