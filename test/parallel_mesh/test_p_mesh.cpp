#include <iostream>
#include <cmath>
#include <string>

#include "meshtools.h"
#include "mesh.h"
#include "parallel_mesh.h"


using namespace std;


int main(int argc, char *argv[])
{
    MeshTools::Init(argc, argv);

    std::unique_ptr<ParallelMesh> mesh = MeshTools::read(std::string(MESHTOOLS_SOURCE_DIR)+"/test/io/ascii/quad_4x4.msh");

    mesh->write_vtk("quad4");

    MeshTools::Finalize();
    return 0;
}