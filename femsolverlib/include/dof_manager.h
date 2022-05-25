
#ifndef DOF_MANAGER_H
#define DOF_MANAGER_H

#include "dirichlet_boundary.h"
#include "parallel_mesh.h"

// dof: graus de liberdade da equação (por nó)
class DofManager
{
public:
    DofManager(ParallelMesh &mesh);
    ~DofManager();
    unsigned int get_n_dofs() { return this->_ndof; };
    void         set_n_dofs(unsigned int n){this->_ndof = n;};
    unsigned int first_global_dof_index();
    unsigned int last_global_dof_index();
    unsigned int n_local_equations();
    /**
     * @brief 
     * 
     * @param id_dof      id da variavel
     * @param local_dof   numeraçao local no elemento (conectividade)
     * @param global_dof  numeracao global das equacoes para o id_dof
     */
    void         dof_indices(int id_dof, unsigned int *local_dof, unsigned int *global_dof);
    void         add_dirichlet_boundary(DirichletBoundary &boundary);
    void         prepare_to_use();
    void         calculate_dnnz_onnz(std::vector<unsigned int> &dnnz, std::vector<unsigned int> &onnz)
    std::vector<int>& get_dof_indices() { return this->_dof_indices; };

private:
    // Private 
    ParallelMesh &              _mesh;
    unsigned int                _ndof;
    std::vector<unsigned int>   _dofs; // global dofs numbering
    unsigned int                _first_global_dof_index; 
    unsigned int                _last_global_dof_index;
    bool                        _prepared_to_use;
    unsigned int                _n_local_equations;


    std::vector<DirichletBoundary> _boundaries;
    std::vector<int>               _dof_indices; // maps global to local indices nnos*ndof
    

};

#endif /* DOF_MANAGER_H */
