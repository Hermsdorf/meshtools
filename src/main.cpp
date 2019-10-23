
#include <iostream>

#include "mesh.h"

int main(int argc, char* argv[])
{
    mesh_t* mesh = MeshGMSHReader("test/test2d_simple.msh");
    MeshVTKWriter(mesh, "test0.vtu");
    MeshVTKWriterInternal(mesh, "test1.vtu");
    delete mesh;
    return 0;
}


