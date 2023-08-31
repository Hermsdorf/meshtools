
#ifndef IMPLICIT_SYSTEM_H
#define IMPLICIT_SYSTEM_H

#include "equation_manager.h"
#include "petsc.h"


class ImplicitSystem 
{
    public:
        ImplicitSystem(ParallelMesh &mesh, std::string name);
        int     add_variable(std::string name);
        std::string  get_variable_name(int id);
        int     get_variable_id(std::string name);
        void    add_dirichlet_boundary(DirichletBoundary &boundary);
        void    init();
        void    close();
        void    add_matrix_entry(std::vector<int>& row_indices, std::vector<int>& col_indices, double* values);
        void    add_matrix_entry(int nrows, int *row_indices, int ncols, int* col_indices, double* values);
        void    add_rhs_entry(std::vector<int>& row_indices, double *value);
        void    add_rhs_entry(int nrows, int* row_indices, double *value);
        void    set_matrix_entry(std::vector<int>& row_indices, std::vector<int>& col_indices, double* values);
        void    set_rhs_entry(std::vector<int>& row_indices, double *value);
        double* get_local_solution_array();
        void    restore_local_solution_array(double** solution_array);
        void    write_result(std::string filename);
        //void    write_xdmf(std::string filename);
        double  compute_error_from_exact_solution(int idof, double(*func_exac)(double x,double y, double z, double t) );
        
        void    apply_dirichlet_boundary_conditions();
        void    print_matrix();
        void    print_rhs();

        const ParallelMesh& get_mesh();
        EquationManager& get_equation_manager();
        
        void   attach_assemble(void _assemble(ImplicitSystem*) );
        
        void   solve();

        void set_linear_tolerance(double tol);

        double get_linear_final_residual();

        ~ImplicitSystem();
    protected:
        
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
        double                   _linear_tolerance;
        void   solve_linear_system();
        void   write_hdf5(unsigned int nfile);
        
    private:

        void (* _assemble_function)(ImplicitSystem*  _system);

};

#endif