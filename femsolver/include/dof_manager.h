
#ifndef ACB619A8_16AD_4AD3_B738_DD90955B29D5
#define ACB619A8_16AD_4AD3_B738_DD90955B29D5

#include "dirichlet_boundary.h"
#include "parallel_mesh.h"

class DofManager
{
public:
    DofManager(const ParallelMesh &mesh);
    unsigned int set_n_dofs();
    void set_dofs_ids(std::vector<unsigned int> &dofs_ids);
    unsigned int first_global_dof_index();
    unsigned int last_global_dof_index();
    void dof_indices(int size, unsigned int *local_dof, unsigned int *global_dof);
    void add_dirichlet_boundary(DirichletBoundary boundary);

private:
    const ParallelMesh &_mesh;
    unsigned int _ndof;
    std::vector<unsigned int> _dofs;
    unsigned int _first_global_dof_index;
    unsigned int _last_global_dof_index;

    std::vector<DirichletBoundary> boundaries;
};

#endif /* ACB619A8_16AD_4AD3_B738_DD90955B29D5 */
