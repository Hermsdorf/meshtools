
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

    ImplicitSystem system(*mesh,"system");
    system.add_variable("u");
    InitialCondition   ic(3,0,"x*y*(x-1)*(y-1)","x,y,z");
    system.add_initial_condition(ic);
    system.init();
    CatalystAdaptor::Execute(0, 0.0, system.get());
    CatalystAdaptor::Finalize();

    if(mesh) delete mesh;
    MeshTools::Finalize();    
    return 0;
}

