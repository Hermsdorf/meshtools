
#ifndef DOF_MANAGER_H
#define DOF_MANAGER_H

#include "dirichlet_boundary.h"
#include "parallel_mesh.h"

class DofManager
{
public:

    DofManager(ParallelMesh &mesh);
    unsigned int get_n_dofs();
    void         add_dof_id(unsigned int dof_id);
    unsigned int first_global_dof_index();
    unsigned int last_global_dof_index();
    void         dof_indices(int size, unsigned int *local_dof, unsigned int *global_dof);
    void         add_dirichlet_boundary(DirichletBoundary &boundary);
    void         prepare_to_use();

private:
    // Private 
    ParallelMesh &              _mesh;
    unsigned int                _ndof;
    std::vector<unsigned int>   _dofs; // global dofs numbering
    unsigned int                _first_global_dof_index; 
    unsigned int                _last_global_dof_index;
    bool                        _prepared_to_use;

    std::vector<DirichletBoundary> _boundaries;
    std::vector<int>               _dof_indices; // maps global to local indices
    

};

#endif /* DOF_MANAGER_H */
