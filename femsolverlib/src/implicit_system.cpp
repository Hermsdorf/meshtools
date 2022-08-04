#include <vector>
#include <algorithm>
using namespace std;

#include "implicit_system.h"
#include "meshtools.h"

ImplicitSystem::ImplicitSystem(ParallelMesh &mesh, std::string name):
    _mesh(mesh), _system_name(name), _n_dof(0), _equations(mesh)
    {

    }   

int ImplicitSystem::add_variable(std::string name)
{
    if(std::find(this->_variables_names.begin(), this->_variables_names.end(), name) == this->_variables_names.end())
    {
        this->_variables_names.push_back(name);
        _n_dof++;
    }
    return this->_variables_names.size() - 1;
}

void ImplicitSystem::add_dirichlet_boundary(DirichletBoundary &boundary)
{
    this->_equations.add_dirichlet_boundary(boundary);
}


void ImplicitSystem::init()
{
    this->_equations.set_n_dofs(this->_variables_names.size());
    
    this->_equations.prepare_to_use();

    std::vector<unsigned int> onnz(this->_equations.n_local_equations());
    std::vector<unsigned int> dnnz(this->_equations.n_local_equations());

    this->_equations.calculate_dnnz_onnz(dnnz, onnz);

    if(MeshTools::n_processors() == 1)
    {
        MatCreateSeqAIJ(MeshTools::Comm(), this->_equations.n_local_equations(), this->_equations.n_local_equations(),
                        PETSC_DECIDE, (PetscInt*) dnnz.data(), &this->_A);
    } 
    else
    {
        // Create the matrix
        MatCreateAIJ(MeshTools::Comm(), 
                this->_equations.n_local_equations(), this->_equations.n_local_equations(),
                 PETSC_DETERMINE, PETSC_DETERMINE, 
                 PETSC_DECIDE, (PetscInt*) dnnz.data(), 
                 PETSC_DECIDE, (PetscInt*) onnz.data(), &this->_A);

    }

    
    MatSetOption(_A, MAT_NEW_NONZERO_ALLOCATION_ERR, PETSC_FALSE);

    MatZeroEntries(_A);
    // Create the right-hand-side vector
    VecCreate(MeshTools::Comm(), &this->_rhs);
    VecSetSizes(this->_rhs, this->_equations.n_local_equations(), PETSC_DETERMINE);
    VecSetFromOptions(this->_rhs);   

    // Create the solution vector
    VecDuplicate(this->_rhs, &this->_solution);

    IS is_local;
    IS is_global;
  
    std::vector<unsigned int> eq_local;
    std::vector<unsigned int> eq_global;
    unsigned int n_nodes = this->_mesh.get_n_nodes();

   for(int ino = 0; ino < n_nodes; ino++)
   {
         for(int idof = 0; idof < this->_variables_names.size(); idof++)
         {
              unsigned int idxLocal = ino*this->_n_dof + idof;
              eq_local.push_back(idxLocal);
              eq_global.push_back(this->_equations.get_equation_indices()[idxLocal]);
         }
   }

    ISCreateGeneral(MeshTools::Comm(), eq_local.size() , (PetscInt *)eq_local.data() , PETSC_COPY_VALUES, &is_local);
    ISCreateGeneral(MeshTools::Comm(), eq_global.size(), (PetscInt *)eq_global.data(), PETSC_COPY_VALUES, &is_global);

    VecCreateSeq(PETSC_COMM_SELF, n_nodes*_n_dof, &_solution_local);
    VecScatterCreate(_solution, is_global, _solution_local, is_local, &_scatter);

    ISDestroy(&is_local);
    ISDestroy(&is_global);

    // Create the KSP solver
    KSPCreate(MeshTools::Comm(), &this->_ksp);
    KSPSetOperators(this->_ksp, this->_A, this->_A);
    KSPSetType(this->_ksp, KSPGMRES);
    KSPSetTolerances(this->_ksp, 1e-12, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT);
    KSPSetFromOptions(this->_ksp);
}

void ImplicitSystem::close()
{
    MatAssemblyBegin(_A,MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_A,MAT_FINAL_ASSEMBLY);
    VecAssemblyBegin(this->_rhs);
    VecAssemblyEnd(this->_rhs);
}

ImplicitSystem::~ImplicitSystem()
{
    KSPDestroy(&this->_ksp);
    VecDestroy(&this->_solution);
    VecDestroy(&this->_rhs);
    MatDestroy(&this->_A);
    VecDestroy(&this->_solution_local);
    VecScatterDestroy(&this->_scatter);
}

void ImplicitSystem::solve()
{

    MatAssemblyBegin(_A,MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_A, MAT_FINAL_ASSEMBLY);
    VecAssemblyBegin(this->_rhs);
    VecAssemblyEnd(this->_rhs);

    this->apply_dirichlet_boundary_conditions();

    KSPSetUp(this->_ksp);
    KSPSolve(this->_ksp, this->_rhs, this->_solution);

    int its;
    double rnorm;
    KSPGetIterationNumber(this->_ksp,&its);
    KSPGetResidualNorm(this->_ksp, &rnorm);
     if(MeshTools::processor_id() == 0) std::cout << "Solution computed" << std::endl;
    PetscPrintf(MeshTools::Comm(), "Number of iterations = %d\n", its);
    PetscPrintf(MeshTools::Comm(), "Final norm of residual: %g\n", rnorm);

    VecScatterBegin(this->_scatter, this->_solution, this->_solution_local, INSERT_VALUES, SCATTER_FORWARD);
    VecScatterEnd(this->_scatter, this->_solution, this->_solution_local, INSERT_VALUES, SCATTER_FORWARD);


}


double ImplicitSystem::compute_error_from_exact_solution( int idof, double(*func_exac)(double x,double y, double z, double t) )
{
    auto coords  = this->_mesh.getCoord();
    auto eqIndex = this->_equations.get_equation_indices();
    int start, end;

    Vec r;
    VecDuplicate(this->_solution, &r);
    VecGetOwnershipRange(this->_solution, &start, &end);
    for(int ino = 0; ino < this->_mesh.get_n_nodes(); ino++)
    {

        double valor = func_exac(coords[ino*3], coords[ino*3+1],0.0, 0.0);
        unsigned int idxLocal = eqIndex[ino*this->_n_dof + idof];
        if(idxLocal >= start && idxLocal < end)
                VecSetValue(r, idxLocal, valor, INSERT_VALUES);

    }

    double _error = 0.0;
    VecAssemblyBegin(r);
    VecAssemblyEnd(r);
    VecAXPY(r, -1.0, this->_solution);
    VecNorm(r, NORM_2, &_error);
    VecDestroy(&r);
    return _error;
}
        

void ImplicitSystem::add_matrix_entry(std::vector<int>& row_indices, 
                                     std::vector<int>& col_indices, double* values)
{
    MatSetValues(this->_A,row_indices.size(),&row_indices[0], col_indices.size(), &col_indices[0],values,ADD_VALUES);
}

void ImplicitSystem::add_matrix_entry(int nrows, int *row_indices, 
                              int ncols, int* col_indices, double* values)
{
    MatSetValues(this->_A,nrows,row_indices, ncols, col_indices,values,ADD_VALUES);
}


void ImplicitSystem::set_matrix_entry(std::vector<int>& row_indices, 
                                      std::vector<int>& col_indices, double* values)
{
    MatSetValues(this->_A,row_indices.size(),row_indices.data(), col_indices.size(), col_indices.data(),values,INSERT_VALUES);
}   

void ImplicitSystem::add_rhs_entry(std::vector<int>& row_indices, double* values)
{
    VecSetValues(this->_rhs, row_indices.size(), row_indices.data(), values, ADD_VALUES);
}

void ImplicitSystem::add_rhs_entry(int nrows, int* row_indices, double* values)
{
    VecSetValues(this->_rhs, nrows, row_indices, values, ADD_VALUES);
}

void ImplicitSystem::set_rhs_entry(std::vector<int>& row_indices, double* values)
{
    VecSetValues(this->_rhs, row_indices.size(), row_indices.data(), values, INSERT_VALUES);
}

double* ImplicitSystem::get_local_solution_array()
{
    double* solution_array;
    VecGetArray(this->_solution_local, &solution_array);
    
    return solution_array;
}

void ImplicitSystem::restore_local_solution_array(double** solution_array)
{
    VecRestoreArray(this->_solution_local, solution_array);
}

EquationManager& ImplicitSystem::get_equation_manager()
{
    return this->_equations;
}

void ImplicitSystem::print_matrix()
{
    MatInfo info;
    PetscBool is_assembled;
    MatAssembled(this->_A,&is_assembled);
    if(!is_assembled)
    {
        MatAssemblyBegin(this->_A,MAT_FINAL_ASSEMBLY);
        MatAssemblyEnd(this->_A,MAT_FINAL_ASSEMBLY);
    }

    MatGetInfo(this->_A, MAT_LOCAL, &info);
    PetscPrintf(PETSC_COMM_WORLD, "Matrix nonzeros: %d\n", info.nz_used);
    PetscPrintf(PETSC_COMM_WORLD, "Matrix nonzeros/proc: %d\n", info.nz_allocated);
    PetscPrintf(PETSC_COMM_WORLD, "Matrix memory: %d\n", info.memory);

    MatView(this->_A, PETSC_VIEWER_STDOUT_WORLD);
    
}

void ImplicitSystem::print_rhs()
{
    VecAssemblyBegin(this->_rhs);
    VecAssemblyEnd(this->_rhs);
    VecView(this->_rhs, PETSC_VIEWER_STDOUT_WORLD);
}

void ImplicitSystem::apply_dirichlet_boundary_conditions()
{

    std::vector<unsigned int>& node_ids = _mesh.getNodeIndexes();

    int nbc = this->_equations.get_number_of_dirichlet_boundaries();
    for(int ibc = 0; ibc < nbc; ibc++)
    {
        if(MeshTools::processor_id() == 0)   std::cout << "Applying dirichlet boundary condition " << ibc << std::endl;
        DirichletBoundary &bc            = this->_equations.get_dirichlet_boundary(ibc);
        std::vector<unsigned int>& nodes = this->_equations.get_boundary_nodes(ibc);
        std::vector<PetscScalar> values(nodes.size());
        std::vector<PetscInt> idx(nodes.size());
        int dof = bc.get_dof_id();
        for(unsigned int inode = 0; inode < nodes.size(); inode++)
        {
            // TODO: usar fparser para avaliar o valor da condicao de contorno
            values[inode] = 0.0;
            idx[inode]    = _n_dof*node_ids[nodes[inode]] + dof;
        }

        MatZeroRows(this->_A, nodes.size(), &idx[0],1.0, 0, 0);
        VecSetValues(this->_rhs, nodes.size(), &idx[0], &values[0], INSERT_VALUES);

    }

    this->close();

}

void ImplicitSystem::write_vtk(string filename)
{
    //FIXME: implementar para sistemas com mais graus de liberdade
    double *solution_ptr = get_local_solution_array();
    MeshIODataAppended info;
    std::string var = this->_variables_names[0];
    info.addPointDataInfo(var.c_str(), Float64, solution_ptr);
    this->_mesh.writePVTK(filename.c_str(), &info);
    restore_local_solution_array(&solution_ptr);
}
