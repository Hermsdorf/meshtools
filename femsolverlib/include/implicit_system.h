
#include "equation_manager.h"
#include "petsc.h"

// Ax = b
class ImplicitSystem 
{
    public:
        ImplicitSystem(ParallelMesh &mesh, std::string name);
        int add_variable(std::string name);
        void add_dirichlet_boundary(DirichletBoundary &boundary);
        void init();
        void close();
        void add_matrix_entry(std::vector<int>& row_indices,std::vector<int>& col_indices, double* values);
        void add_matrix_entry(int nrows, int *row_indices, int ncols, int* col_indices, double* values);
        void add_rhs_entry(std::vector<int>& row_indices, double *value);
        void add_rhs_entry(int nrows, int* row_indices, double *value);
        void set_matrix_entry(std::vector<int>& row_indices, std::vector<int>& col_indices, double* values);
        void set_rhs_entry(std::vector<int>& row_indices, double *value);
        double* get_local_solution_array();
        void restore_local_solution_array(double** solution_array);

        void print_matrix();
        void print_rhs();

        EquationManager& get_equation_manager();
        
        //void assemble();
        void solve();
        ~ImplicitSystem();
    private:
        std::vector<std::string> _variables_names;
        std::string              _system_name;
        unsigned int             _n_dof; 
        Vec                      _rhs;
        Vec                      _solution;
        Mat                      _A;
        Vec                      _solution_local;
        KSP                      _ksp;
        EquationManager          _equations;
        ParallelMesh&            _mesh;
        VecScatter               _scatter;

};