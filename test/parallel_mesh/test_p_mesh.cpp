#include <iostream>
#include <cmath>

#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"

using namespace std;

int main(int argc, char* argv[])
{
    Mesh* mesh                  = new Mesh();
    MeshPartition * partitioner = new MeshPartition();
    mesh->MeshGmshReader(argv[1]);
   

    int nprocs = 4;
    partitioner->ApplyPartitioner(mesh,nprocs,false);
     
    partitioner->WriteAscii(mesh,nprocs, "serial");
    partitioner->WriteVTK(mesh,"vtk");
    partitioner->WriteDistributedMesh(mesh,0,nprocs,"parallel");

    if(mesh) delete mesh;
    if(partitioner) delete partitioner;
    
    return 0;
}