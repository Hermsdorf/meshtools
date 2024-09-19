#include "meshtools.h"
#include "parallel_mesh.h"
#include "mesh_refinement.h"
#include "gmsh_io.h"

int main(int argc, char *argv[])
{
  
    MeshTools::Init(argc, argv);

    std::unique_ptr<ParallelMesh> mesh = MeshTools::read(std::string(MESHTOOLS_SOURCE_DIR)+"/test/io/ascii/tet4.msh");

    mesh->write_vtk("mesh_original");

    MeshRefinement refiner(mesh);

    for(int r = 0; r < 2; r++)
        refiner.refine();
   
        
    mesh->write_vtk("mesh_refined");

    MeshTools::Finalize();
    return 0;
}
