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
     delete [] (*mesh)->mesh_coloring_internal;
     delete *mesh;
}

int* GetElementConn(mesh_t* mesh, int element_num)
{
    if(element_num < mesh->n_elements)
    {
        return &mesh->conn[mesh->offset[element_num + mesh->n_face_elements]];
    }
    else
    {
        cout << "ERROR: element number >= n_elements" << endl;
        exit(1);
    }
    
} // element_num = [0, n_elements);

int* GetElementOffset(mesh_t* mesh, int element_num)
{
    if(element_num < mesh->n_elements)
    {
        return &mesh->offset[element_num + mesh->n_face_elements];
    }
    else
    {
        cout << "ERROR: element number >= n_elements" << endl;
        exit(1);
    }
    
} 

int* GetSurfaceElementConn(mesh_t* mesh, int element_num)
{
    if(element_num < mesh->n_face_elements)
    {
        return &mesh->conn[mesh->offset[element_num]];
    }
    else
    {
        cout << "ERROR: element number >= n_face_elements" << endl;
        exit(1);
    }
    
} // element_num = [0, n_surface_elements);

int* GetSurfaceElementOffset(mesh_t* mesh, int element_num)
{
    if(element_num < mesh->n_face_elements)
    {
        return &mesh->offset[element_num];
    }
    else
    {
        cout << "ERROR: element number >= n_face_elements" << endl;
        exit(1);
    }
}

int GetElementConnSize(mesh_t* mesh, int element_num)
{
    if(element_num < mesh->n_elements)
    {
        return (mesh->offset[mesh->n_face_elements + element_num + 1] - mesh->offset[mesh->n_face_elements + element_num]);
    }
    else
    {
        cout << "ERROR: element number >= n_elements" << endl;
        exit(1);
    }
    
}

int GetSurfaceElementConnSize(mesh_t* mesh, int element_num)
{
    if(element_num < mesh->n_face_elements)
    {
        return (mesh->offset[element_num + 1] - mesh->offset[element_num]);
    }
    else
    {
        cout << "ERROR: element number >= n_face_elements" << endl;
        exit(1);
    }
}