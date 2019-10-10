
#include <iostream>

#include "mesh.h"


int main(int argc, char* argv[])
{
    mesh_t* mesh = MeshGMSHReader("test/test2d_simple.msh");

    delete mesh;
    return 0;
}


