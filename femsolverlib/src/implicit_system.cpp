#include <vector>
#include <algorithm>
using namespace std;

#include "implicit_system.h"
#include "meshtools.h"

ImplicitSystem::ImplicitSystem(ParallelMesh &mesh, std::string name):
    mesh(mesh), system_name(name), n_dof(0), equations(mesh)
    {
        //this->init();
    }   

int ImplicitSystem::addVariable(std::string name)
{
    n_dof++;

    if(std::find(this->variables_names.begin(), this->variables_names.end(), name) != this->variables_names.end())
        this->variables_names.push_back(name);

    return this->variables_names.size() - 1;
}



void ImplicitSystem::init()
{
 
    this->equations.set_n_dofs(this->variables_names.size());
    this->equations.prepare_to_use();

    std::vector<unsigned int> onnz(this->equations.n_local_equations());
    std::vector<unsigned int> dnnz(this->equations.n_local_equations());
    this->equations.calculate_dnnz_onnz(dnnz, onnz);

    // Create the matrix
    MatCreateAIJ(MeshTools::Comm(), this->equations.n_local_equations(), this->equations.n_local_equations(),
                 PETSC_DETERMINE, PETSC_DETERMINE, NULL,
                 (PetscInt*) dnnz.data(), NULL, (PetscInt*) onnz.data(), &this->A);

    // Create the right-hand-side vector
    VecCreate(MeshTools::Comm(), &this->rhs);
    VecSetSizes(this->rhs, this->equations.n_local_equations(), PETSC_DETERMINE);
    VecSetFromOptions(this->rhs);   

    // Create the solution vector
    VecDuplicate(this->rhs, &this->solution);

    IS is_local;
    IS is_global;
  
    std::vector<unsigned int> eq_local;
    std::vector<unsigned int> eq_global;
    unsigned int n_nodes = this->mesh.get_n_nodes();

   for(int ino = 0; ino < n_nodes; ino++)
   {
         for(int idof = 0; idof < this->variables_names.size(); idof++)
         {
              unsigned int idxLocal = ino*this->n_dof + idof;
              eq_local.push_back(idxLocal);
              eq_global.push_back(this->equations.get_dof_indices()[idxLocal]);
         }
   }

    ISCreateGeneral(MeshTools::Comm(), eq_local.size(), (PetscInt *)eq_local.data(), PETSC_COPY_VALUES, &is_local);
    ISCreateGeneral(MeshTools::Comm(), eq_global.size(), (PetscInt *)eq_global.data(), PETSC_COPY_VALUES, &is_global);

    VecCreateSeq(PETSC_COMM_SELF, n_nodes*n_dof, &solution_local);
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