
#include "meshtools.h"
#include "mesh.h"

#include "gmsh_io.h"

int main(int argc, char* argv[])
{
    MeshTools::Init(argc, argv);
    
    Mesh mesh;
    mesh.read(std::string(MESHTOOLS_SOURCE_DIR)+"/test/io/ascii/tri3.msh");

    mesh.write_glvis("tri3.mesh");

    GmshIO::write("out_tri3.msh", mesh);

    MeshTools::Finalize();
}


