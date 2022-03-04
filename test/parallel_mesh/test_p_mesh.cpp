#include <iostream>
#include <cmath>
#include <string>

#include "meshtools.h"
#include "mesh.h"
#include "mesh_part.h"
#include "parallel_mesh.h"
#include "GetPot.hpp"


using namespace std;

void PrintMenu(std::string exec_name)
{
    std::cout<<"\nUsage: ./" << exec_name <<" [options] \n"
             <<"  -h             : print help menu    \n"
             <<"  -m [gmsh file] : read gmsh file     \n";
}

int main(int argc, char* argv[])
{
    MeshTools::Init(argc, argv);
    GetPot cl(argc, argv);
 
    Mesh             *mesh        = nullptr;
    ParallelMesh     *pmesh       = nullptr;
    MeshPartition    *partitioner = new MeshPartition();

    if(cl.size() == 1 || cl.search("-h"))
    {
        if(MeshTools::processor_id() == 0 )
            PrintMenu(cl[0]);
             
        MeshTools::Finalize();
        return 0;
    }

    if(!cl.search("-m"))
    {
        if(MeshTools::processor_id() == 0 )
                PrintMenu(argv[0]);
             
        MeshTools::Finalize();
        return 0;
    }

    const string filename = cl.next(std::string(""));

    if(MeshTools::processor_id() == 0 )
    {
        mesh                  = new Mesh();
        mesh->MeshGmshReader(filename.c_str());
        mesh->WriteVTK("serial");
    }
    
    pmesh = partitioner->DistributedMesh(mesh);
    {   
        MeshIODataAppended info;
        auto& l2g = pmesh->getLocal2Global();
        info.addPointDataInfo("Index",UInt32,(void*)&l2g[0]);
        //pmesh->renumbering();
        pmesh->writePVTK("parallel", &info);
        pmesh->WritePMesh("mesh");
    }

    if(mesh)        delete mesh;
    if(partitioner) delete partitioner;
    if(pmesh)       delete pmesh;
    
    MeshTools::Finalize();
    return 0;
}