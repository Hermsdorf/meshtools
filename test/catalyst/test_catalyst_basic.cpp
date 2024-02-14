
#include <iostream>

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"
#include "implicit_system.h"
#include "transient_implicit_system.h"
#include "dirichlet_boundary.h"
#include "catalyst_adaptor.h"

int main(int argc, char* argv[])
{
    MeshTools::Init(argc,argv);
    CatalystAdaptor::Initialize(argc,argv);
    
    ParallelMesh* mesh = MeshTools::ReadMesh("catalyst_mesh.msh");

    TransientImplicitSystem *system  = new TransientImplicitSystem(*mesh,"system");
    system->add_variable("u");
    InitialCondition   ic(2,0,"x*y*(x-1)*(y-1)","x,y,z");
    system->add_initial_condition(ic);
    system->init();
    system->write_result("result");
    CatalystAdaptor::Execute(0, 0.0,system);
    CatalystAdaptor::Finalize();

    if(mesh) delete mesh;
    MeshTools::Finalize();    
    return 0;
}

