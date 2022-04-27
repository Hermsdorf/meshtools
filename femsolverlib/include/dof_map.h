#ifndef D0774C44_AF34_4DE0_924C_560AEE28018A
#define D0774C44_AF34_4DE0_924C_560AEE28018A

#include "meshtools.h"
#include "parallel_mesh.h"


class DofMap {
    
    public:
        DofMap(const ParallelMesh& pmesh);
        
        void add_dirichlet_bc();
        void dof_indices();
        // Vsi teri que considerar CC?
        void compute_nnz();
        
        
    private:
        unsigned int              n_dofs;
        std::vector<unsigned int> dofs;
      
};

#endif /* D0774C44_AF34_4DE0_924C_560AEE28018A */
