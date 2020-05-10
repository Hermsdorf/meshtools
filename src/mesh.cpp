#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <algorithm>

using namespace std;
#include "mesh.h"


mesh_t* MeshCreate()
{
    mesh_t* mesh = new mesh_t();
    mesh->n_face_elements = 0;
    mesh->n_elements = 0;
    mesh->n_nodes = 0; 
    mesh->dim = 0;
    return mesh;
}

void MeshDestroy(mesh_t** mesh)
{
     (*mesh)->conn.clear();
     (*mesh)->coord.clear();
     (*mesh)->offset.clear();
     (*mesh)->type.clear();
     (*mesh)->physical_tag.clear();
     (*mesh)->physical_map.clear();
     delete [] (*mesh)->mesh_coloring;
     delete *mesh;
}
