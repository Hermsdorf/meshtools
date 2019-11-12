
#include <iostream>

#include "mesh.h"

int main(int argc, char* argv[])
{


    mesh_t* mesh            = MeshGMSHReader("test/test2d_simple.msh");
    mesh_partition_t *parts = MeshPartitioner(mesh, 4);

    MeshVTKWriterInternal(mesh, "test1.vtu", parts->nodal_part, parts->elem_part);

    MeshPartitionDestroy(parts);
    delete mesh;
    return 0;
}


