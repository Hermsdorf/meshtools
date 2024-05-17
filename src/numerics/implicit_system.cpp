#include <vector>
#include <algorithm>
using namespace std;

#include "implicit_system.h"
#include "meshtools.h"

/**
 * Constructor
 * 
 * @param mesh: parallel mesh object
 * @param name: name of the system
*/
ImplicitSystem::ImplicitSystem(ParallelMesh &mesh, std::string name):
    _mesh(mesh), _system_name(name), _n_dof(0), _equations(mesh), _assemble_function(nullptr)
    {

    }   

/**
 * Add a variable to the system
 * 
 * @param name: name of the variable
 * @return the number of variables in the system
*/
int ImplicitSystem::add_variable(std::string name)
{
    if(std::find(this->_variables_names.begin(), this->_variables_names.end(), name) == this->_variables_names.end())
    {
        this->_variables_names.push_back(name);
        _n_dof++;
    }
    return this->_variables_names.size() - 1;
}


/**
 * Get the name of a variable
 * 
 * @param idx: index of the variable
 * @return the name of the variable
*/
std::string ImplicitSystem::get_variable_name(int idx)
{
    assert(idx < this->_variables_names.size());
    return this->_variables_names[idx];
}

/**
 * Get the index of a variable
 * 
 * @param name: name of the variable
 * @return the index of the variable. 
 * If the variable is not found, it returns -1
*/
int ImplicitSystem::get_variable_id(std::string name)
{
    for(int i = 0; i < this->_variables_names.size();i++)
        if( this->_variables_names[i]==name)
            return i;
    return -1;
}

/**
 * Add a Dirichlet boundary condition to the system
 * 
 * @param boundary: Dirichlet boundary condition
*/
void ImplicitSystem::add_dirichlet_boundary(DirichletBoundary &boundary)
{
    this->_equations.add_dirichlet_boundary(boundary);
}


/**
 * Initializes the system setting the number of degrees of freedom,
 * alocating the matrix, the vector and the solver with its tolerance,
 * and type of solver, which is GMRES.
*/
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

    VecSet(this->_solution, 0.0);
    VecSet(this->_solution_local, 0.0);

    // Create the KSP solver
    KSPCreate(MeshTools::Comm(), &this->_ksp);
    KSPSetOperators(this->_ksp, this->_A, this->_A);
    KSPSetType(this->_ksp, KSPGMRES);
    KSPSetFromOptions(this->_ksp);
}

/**
 * Update the matrix and the right-hand-side vector
 * in all processors that are executing the code
*/
void ImplicitSystem::close()
{
    MatAssemblyBegin(_A,MAT_FINAL_ASSEMBLY);
    MatAssemblyEnd(_A,MAT_FINAL_ASSEMBLY);
    VecAssemblyBegin(this->_rhs);
    VecAssemblyEnd(this->_rhs);
}

/**
 * Destructor of the class.
*/
ImplicitSystem::~ImplicitSystem()
{
    KSPDestroy(&this->_ksp);
    VecDestroy(&this->_solution);
    VecDestroy(&this->_rhs);
    MatDestroy(&this->_A);
    VecDestroy(&this->_solution_local);
    VecScatterDestroy(&this->_scatter);
}

/**
 * Solves the linear system using the KSP solver.
 * It assembles the system using the assemble function
 * passed as parameter to the constructor and implemented
 * by the user and then solves the linear system with PETSc.
*/
void ImplicitSystem::solve()
{
    this->_assemble_function(this);
    solve_linear_system();
}

/**
 * Solves the linear system using the KSP solver.
 * It assemble the matrix and the right-hand-side vector
 * and then solves the linear system with PETSc. By the end
 * of the function, the solution is scattered from global to local
 * solution vector.
*/
void ImplicitSystem::solve_linear_system()
{ 
    MatAssemblyBegin(_A, MAT_FINAL_ASSEMBLY);
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
    PetscPrintf(MeshTools::Comm(), "Linear number of iterations = %d\n", its);
    PetscPrintf(MeshTools::Comm(), "Linear final norm of residual: %g\n", rnorm);

    // Realiza o scatter de solução global para local
    VecScatterBegin(this->_scatter, this->_solution, this->_solution_local, INSERT_VALUES, SCATTER_FORWARD);
    VecScatterEnd(this->_scatter  , this->_solution, this->_solution_local, INSERT_VALUES, SCATTER_FORWARD);
}

/**
 * @brief Set the linear tolerance
 * 
 * @param tol 
 */
void ImplicitSystem::set_linear_tolerance(double tol)
{
    this->_linear_tolerance = tol;
    //KSPSetTolerances(this->_ksp, tol, PETSC_DEFAULT, PETSC_DEFAULT, PETSC_DEFAULT);
}

double ImplicitSystem::get_linear_final_residual()
{
    double rnorm;
    KSPGetResidualNorm(this->_ksp, &rnorm);
    return rnorm;
}

/**
 * By a exact solution, it computes the error of the solution
 * in the L2 norm.
 * 
 * @param idof: Degree of freedom to compute the error
 * @param func_exac: Exact solution
 * @return The error in the L2 norm
*/
double ImplicitSystem::compute_error_from_exact_solution( int idof, double(*func_exac)(double x,double y, double z, double t) )
{
    auto &coords  = this->_mesh.get_coordinate_vector();
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
        
/**
 * It adds values in the matrix by std::vectors of row and column indices
 * 
 * @param row_indices: std::vector of row indices
 * @param col_indices: std::vector of column indices
 * @param values: double* of values to be added in the matrix
*/
void ImplicitSystem::add_matrix_entry(std::vector<int>& row_indices, 
                                      std::vector<int>& col_indices, double* values)
{
    MatSetValues(this->_A,row_indices.size(),row_indices.data(), col_indices.size(), col_indices.data(),values,ADD_VALUES);
}

/**
 * It adds values in the matrix by arrays of row and column indices
 * 
 * @param nrows: number of rows
 * @param row_indices: int* with the indices of the rows
 * @param ncols: number of columns
 * @param col_indices: int* with the indices of the columns
 * @param values: double* of values to be added in the matrix
*/
void ImplicitSystem::add_matrix_entry(int nrows, int *row_indices, 
                              int ncols, int* col_indices, double* values)
{
    MatSetValues(this->_A,nrows,row_indices, ncols, col_indices,values,ADD_VALUES);
}

/**
 * It sets values in the matrix by std::vectors of row and column indices
 * 
 * @param row_indices: std::vector of row indices
 * @param col_indices: std::vector of column indices
 * @param values: double* of values to be set in the matrix
*/
void ImplicitSystem::set_matrix_entry(std::vector<int>& row_indices, 
                                      std::vector<int>& col_indices, double* values)
{
    MatSetValues(this->_A,row_indices.size(),row_indices.data(), col_indices.size(), col_indices.data(),values,INSERT_VALUES);
}   

/**
 * It adds to the right-hand-side vector by std::vectors of row indices
 * 
 * @param row_indices: std::vector of row indices
 * @param values: double* of values to be added in the right-hand-side vector
*/
void ImplicitSystem::add_rhs_entry(std::vector<int>& row_indices, double* values)
{
    VecSetValues(this->_rhs, row_indices.size(), row_indices.data(), values, ADD_VALUES);
}


/**
 * It adds to the right-hand-side vector by arrays of row indices
 * 
 * @param nrows: number of rows
 * @param row_indices: int* with the indices of the rows
 * @param values: double* of values to be added in the right-hand-side vector
*/
void ImplicitSystem::add_rhs_entry(int nrows, int* row_indices, double* values)
{
    VecSetValues(this->_rhs, nrows, row_indices, values, ADD_VALUES);
}

/**
 * It sets values in the right-hand-side vector by std::vectors of row indices
 * 
 * @param row_indices: std::vector of row indices
 * @param values: double* of values to be set in the right-hand-side vector
*/
void ImplicitSystem::set_rhs_entry(std::vector<int>& row_indices, double* values)
{
    VecSetValues(this->_rhs, row_indices.size(), row_indices.data(), values, INSERT_VALUES);
}

/**
 * It get the local solution array
 * 
 * @return double* with the local solution array
*/
double* ImplicitSystem::get_local_solution_array()
{
    double* solution_array;
    VecGetArray(this->_solution_local, &solution_array);
    
    return solution_array;
}

/**
 * It restores the local solution array
 * 
 * @param solution_array: double* with the local solution array
*/
void ImplicitSystem::restore_local_solution_array(double** solution_array)
{
    VecRestoreArray(this->_solution_local, solution_array);
}

/**
 * Returns the equation manager
 * 
 * @return EquationManager& with the equation manager
*/
EquationManager& ImplicitSystem::get_equation_manager()
{
    return this->_equations;
}

/**
 * Prints the matrix with the number of nonzero elements,
 * the number of nonzero elements per processor and the memory used
*/
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
    PetscPrintf(PETSC_COMM_WORLD, "Matrix nonzeros: %f\n", info.nz_used);
    PetscPrintf(PETSC_COMM_WORLD, "Matrix nonzeros/proc: %f\n", info.nz_allocated);
    PetscPrintf(PETSC_COMM_WORLD, "Matrix memory: %f\n", info.memory);

    MatView(this->_A, PETSC_VIEWER_STDOUT_WORLD);
    
}

/**
 * Prints the right-hand-side vector
*/
void ImplicitSystem::print_rhs()
{
    VecAssemblyBegin(this->_rhs);
    VecAssemblyEnd(this->_rhs);
    VecView(this->_rhs, PETSC_VIEWER_STDOUT_WORLD);
}

/**
 * Applies dirichlet boundary conditions to the system
 * The rows corresponding to the dirichlet boundary conditions are zeroed
 * and the diagonal is set to 1.0. The rhs is also modified accordingly
 * with the dirichlet boundary condition value.
*/
void ImplicitSystem::apply_dirichlet_boundary_conditions()
{

    auto          & node_ids = _mesh.get_node_index_vector();
    auto          & coords = _mesh.get_coordinate_vector();

    int nbc = this->_equations.get_number_of_dirichlet_boundaries();
    for(int ibc = 0; ibc < nbc; ibc++)
    {
        DirichletBoundary &bc            = this->_equations.get_dirichlet_boundary(ibc);
        std::vector<unsigned int>& nodes = this->_equations.get_boundary_nodes(ibc);
        std::vector<PetscScalar> values(nodes.size());
        std::vector<PetscInt> idx(nodes.size());

        int dof = bc.get_dof_id();
        for(unsigned int inode = 0; inode < nodes.size(); inode++)
        {
            int node_id = nodes[inode];
            double x    = coords[node_id*3];
            double y    = coords[node_id*3+1];
            double z    = coords[node_id*3+2];  

            values[inode] =  bc.get_value(x,y,z);
            idx[inode]    = _n_dof*node_ids[nodes[inode]] + dof;
        }

        MatZeroRows( this->_A  , nodes.size(), &idx[0],1.0        , 0, 0);
        VecSetValues(this->_rhs, nodes.size(), &idx[0], &values[0], INSERT_VALUES);

    }
    this->close();
}

/**
 * Write the solution in a file
 * 
 * @param filename: string with the filename
*/
void ImplicitSystem::write_result(string filename)
{
    auto n_nodes = _mesh.get_n_nodes();
    
    std::vector<double> solution(n_nodes*_n_dof);
    unsigned int offset  = 0;
    double *solution_ptr = get_local_solution_array();
    MeshIODataAppended info;
    for(int i = 0; i < _n_dof; i++)
    {
        for(int ino = 0; ino < n_nodes; ino++)
            solution[ino+offset] = solution_ptr[ino*_n_dof + i];

        std::string var = this->_variables_names[0];
        info.addPointDataInfo(var.c_str(), Float64, &solution[offset]);
        offset += n_nodes;
    }
    //this->_mesh.write(filename.c_str(), &info);
    restore_local_solution_array(&solution_ptr);
}

/**
 * Returns the mesh associated to the system
 * 
 * @return ParallelMesh& with the mesh
*/
ParallelMesh& ImplicitSystem::get_mesh()
{
    return this->_mesh;
}

/**
 * Store the assemble function declared by the user
 * in the system.
 * 
 * @param _assemble: function pointer to the assemble function
 * to be stored in the system
*/
void ImplicitSystem::attach_assemble(void _assemble(ImplicitSystem* _system))
{
    this->_assemble_function = _assemble;
}