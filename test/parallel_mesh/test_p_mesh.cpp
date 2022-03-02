#include <iostream>
#include <cmath>

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"

using namespace std;

int main(int argc, char* argv[])
{
    MeshTools::Init(argc, argv);
 
    Mesh             *mesh        = nullptr;
    ParallelMesh     *pmesh       = nullptr;
    MeshPartition    *partitioner = new MeshPartition();

    if(MeshTools::processor_id() == 0 )
    {
        mesh                  = new Mesh();
        mesh->MeshGmshReader(argv[1]);
    }
    
    partitioner->ApplyPartitioner(mesh,MeshTools::n_processors());
     
    if(MeshTools::processor_id() == 0 ) 
    {
        partitioner->WriteAscii(mesh,MeshTools::n_processors(), "serial");
        partitioner->WriteVTK(mesh,"vtk");
        partitioner->WriteDistributedMesh(mesh,0,MeshTools::n_processors(),"parallel");
    }

    

    pmesh = partitioner->DistributedMesh(mesh);

    if(mesh)        delete mesh;
    if(partitioner) delete partitioner;
    if(pmesh)       delete pmesh;
    
    MeshTools::Finalize();
    return 0;
}