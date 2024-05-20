#include "meshtools.h"
#include "parallel_mesh.h"
#include "mesh_refinement.h"
#include "gmsh_io.h"

int main(int argc, char *argv[])
{
  
    MeshTools::Init(argc, argv);

    std::unique_ptr<ParallelMesh> mesh = MeshTools::read(std::string(MESHTOOLS_SOURCE_DIR)+"/test/io/ascii/quad4.msh");

    mesh->write_vtk("quad4");

    MeshRefinement refiner(mesh);
    refiner.uniform_refinement(1);

    mesh->write_vtk("quad4_refined");

    //GmshIO::write("quad4.msh", *mesh);

    MeshTools::Finalize();
    return 0;
}
