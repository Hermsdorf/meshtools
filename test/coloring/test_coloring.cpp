#include <iostream>

#include "mesh.h"

using namespace std;

int main()
{         
    Mesh* mesh = new Mesh("quadrado.msh");
    Mesh* mesh_trid = new Mesh("trid.msh");
    
    cout << "Testing coloring algorithm in a bidimensional mesh...\n";
    mesh->MeshReordering(RCM);
    mesh->MeshColoring_test();

    cout << "Testing coloring algorithm in a tridimensional mesh...\n";
    mesh_trid->MeshReordering(RCM);
    mesh_trid->MeshColoring_test();
    
    delete mesh;
    delete mesh_trid;
    return 0;
}
