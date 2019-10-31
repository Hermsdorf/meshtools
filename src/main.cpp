
#include <iostream>

#include "mesh.h"

int main(int argc, char* argv[])
{
    mesh_t* mesh = MeshGMSHReader("test/test2d_simple.msh");

    int * epart  = new int [mesh->n_elements];
    int * npart  = new int [mesh->n_nodes]; 

    MeshPartitioner(mesh, 4, npart, epart );
    MeshVTKWriterInternal(mesh, "test1.vtu", npart, epart);

    delete [] epart;
    delete [] npart;

    delete mesh;
    return 0;
}


