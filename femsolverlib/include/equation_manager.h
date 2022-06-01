
#ifndef EQUATION_MANAGER_H
#define EQUATION_MANAGER_H

#include "dirichlet_boundary.h"
#include "parallel_mesh.h"



class EquationManager
{
public:
    EquationManager(ParallelMesh &mesh);
    ~EquationManager();
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
    void         equation_indices(int id_dof, unsigned int *local_dof, unsigned int *global_dof);
    void         add_dirichlet_boundary(DirichletBoundary &boundary);
    void         prepare_to_use();
    void         calculate_dnnz_onnz(std::vector<unsigned int> &dnnz, std::vector<unsigned int> &onnz);
    std::vector<int>& get_dof_indices() { return this->_equation_indices; };

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

    // boundary nodes mapping
    std::vector< std::vector <unsigned int> > _boundary_nodes_map;
    std::vector<int>                          _equation_indices; // maps global to local indices nnos*ndof
    

};

#endif /* EQUATION_MANAGER_H */
