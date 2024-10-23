
#include "meshtools.h"
#include "mesh.h"
#include "mesh_generation.h"

#include "gmsh_io.h"



int main(int argc, char* argv[])
{
    MeshTools::Init(argc, argv);
    
    auto mesh = MeshGeneration::build_square(2, 2, 0.0, 1.0, 0.0, 1.0);
    // Mesh mesh;
    // mesh.read(std::string(MESHTOOLS_SOURCE_DIR)+"/test/io/ascii/quad4.msh");

    // mesh.write_glvis("quad4.mesh");
    
    // mesh.write_vtk("quad4");

    // GmshIO::write("quad4.msh", mesh);

    mesh->print_info(true);

    MeshTools::Finalize();
}
