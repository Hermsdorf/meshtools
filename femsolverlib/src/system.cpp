#include <vector>
using namespace std;

#include "system.h"

ImplicitSystem::ImplicitSystem(ParallelMesh &mesh, std::string name)
    : mesh(mesh), system_name(name), n_ndof(0)
{
    //this->init();
}   

int ImplicitSystem::addVariable(std::string name)
{
    n_ndof++;
    // FIXME: verificar se name ja existe
    this->variables_names.push_back(name);
    return this->variables_names.size() - 1;
}



void ImplicitSystem::init()
{
 
    this->dof.set_n_dofs(this->variables_names.size());
    this->dof.prepare_to_use();

    std::vector<unsigned int> onnz(this->dof.n_local_equations());
    std::vector<unsigned int> dnnz(this->dof.n_local_equations());
    this->dof.calculate_onnz_dnnz(dnnz, onnz);

    // Create the matrix
    MatCreateAIJ(MeshTools::Comm(), this->dof.n_local_equations(), this->dof.n_local_equations(),
                 PETSC_DETERMINE, PETSC_DETERMINE,
                 dnnz.data(), onnz.data(), &this->A);

    // Create the right-hand-side vector
    VecCreate(MeshTools::Comm(), &this->rhs);
    VecSetSizes(this->rhs, this->dof.n_local_equations(), PETSC_DETERMINE);
    VecSetFromOptions(this->rhs);   

    // Create the solution vector
    VecDuplicate(this->rhs, &this->solution);

    IS is_local;
    IS is_global;
  
    std::vector<unsigned int> eq_local;
    std::vector<unsigned int> eq_global;

   for(int ino = 0; ino < this->mesh.n_nodes(); ino++)
   {
         for(int idof = 0; idof < this->variables_names.size(); idof++)
         {
              unsigned int idxLocal = ino*this->_n_dof + idof;
              eq_local.push_back(idxLocal);
              eq_global.push_back(this->dof.get_dof_indices()[idxLocal]);
         }
   }

    ISCreateGeneral(MeshTools::Comm(), eq_local.size(), (PetscInt *)eq_local.data(), PETSC_COPY_VALUES, &is_local);
    ISCreateGeneral(MeshTools::Comm(), eq_global.size(), (PetscInt *)eq_global.data(), PETSC_COPY_VALUES, &is_global);

    VecCreateSeq(PETSC_COMM_SELF, n_nodes*n_dof, &local);
    VecScatterCreate(solution, is_global, solution_local, is_local, &scatter);

    ISDestroy(&is_local);
    ISDestroy(&is_global);

}

void ImplicitSystem::assemble()
{
    VecScatterBegin(scatter, solution, solution_local, INSERT_VALUES, SCATTER_FORWARD);
    VecScatterEnd(scatter  , solution, solution_local, INSERT_VALUES, SCATTER_FORWARD);
}

ImplicitSystem::~ImplicitSystem()
{
    VecDestroy(&this->solution);
    VecDestroy(&this->rhs);
    MatDestroy(&this->A);
    VecDestroy(&this->solution_local);
    VecScatterDestroy(&this->scatter);
}