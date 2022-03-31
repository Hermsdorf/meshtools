#ifndef SYSTEM_H
#define SYSTEM_H

#include "meshtools.h"
#include "parallel_mesh.h"
#include "petsc.h"

class System
{
public:
    System(const ParallelMesh &pmesh, std::string system_name);
    unsigned int add_variable(std::string variable_name);
    unsigned int get_variable_id(std::string variable_name);
    void add_dirichlet_boundary(unsigned int boundary_id,
                                std::string variable_name);

    // deve ser chamada apos definir as variveis e as condições de contorno
    // Conta numero de equacoes locais e aloca as estrutura do sistema.
    void init();

private:
    const ParallelMesh &_pmesh;
    std::string _system_name;
    std::map<std::string, int> _vars;
    int _n_dofs;
    Vec _solution;
    VecScatter _scatter;
};

#endif /* SYSTEM_H */
