
#include "equation_manager.h"
#include "petsc.h"

// Ax = b
class ImplicitSystem 
{
    public:
        ImplicitSystem(ParallelMesh &mesh, std::string name);
        int addVariable(std::string name);
        void init();
        void assemble();
        void solve();
        ~ImplicitSystem();
    private:
        std::vector<std::string> variables_names;
        std::string              system_name;
        int n_dof; 
        Vec          rhs;
        Vec          solution;
        Mat          A;
        Vec          solution_local;
        EquationManager equations;
        ParallelMesh& mesh;
        VecScatter scatter;

};