#ifndef NAVIER_STOKES_H__
#define NAVIER_STOKES_H__

#include <iostream>
#include <cmath>
#include <string>

#include "parallel_mesh.h"
#include "transient_implicit_system.h"


class NavierStokes {
public:
    NavierStokes(ParallelMesh * mesh);
    ~NavierStokes();

    void init();
    void run();
    void finalize();
private:
    void assemble_lhs();
    void assemble_rhs();
    ParallelMesh            * _mesh;
    std::unique_ptr<TransientImplicitSystem> _system;
    std::vector<std::string>  variables;
}


#endif /* NAVIER_STOKES_H__ */
