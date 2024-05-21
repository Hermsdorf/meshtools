#include "meshtools.h"
#include "parallel_mesh.h"
#include "mesh_refinement.h"
#include "gmsh_io.h"

int main(int argc, char *argv[])
{
  
    MeshTools::Init(argc, argv);

    std::unique_ptr<ParallelMesh> mesh = MeshTools::read(std::string(MESHTOOLS_SOURCE_DIR)+"/test/io/ascii/quad_2x2.msh");

    mesh->write_vtk("quad4");

    mesh->print_info(true);

    MeshRefinement refiner(mesh);
    refiner.refine();

    mesh->print_info(true);

    mesh->write_vtk("quad4_refined");

    MeshTools::Finalize();
    return 0;
}
