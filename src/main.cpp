#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include "FEAdaptor.h"

#include "mesh.h"


// TODO: Aqui colocar função de velocidade conhecida 
//     
void UpdateAttr(int n, int time, double*v, float *p)
{
    for(int i = 0; i < n; i++)
    {
        v[i*3]    = time;
        v[i*3+1] = time;
        v[i*3+2] = time;
        p[i]     =  time;
    }
}

int main(int argc, char* argv[])
{      
    if(argc < 3)
    {
        cout << "ERROR: WRONG EXECUTION" << endl;
	    cout << "./meshtools filename.msh n_partitions [catalyst.py]" << endl;

	    return 0;
    }
    string str = argv[1];

    str.resize(str.length()-4);
  
    const char* out;
    str = str.append(".vtu");
    out = str.c_str();
    int n_part = atoi(argv[2]);

    int n_script = 0;
    if(argc == 4)
        n_script = 1;

    CatalystInitialize(n_script, argv+3);
    
    mesh_t* mesh = MeshCreate();
    
    MeshGmshReader(mesh, argv[1]);

    MeshReordering(mesh, RCM);

    MeshColoring(mesh);

    mesh_partition_t *parts = MeshPartitioner(mesh, n_part);

    double *velocity = new double[mesh->n_nodes*3];
    float *pressure  = new float[mesh->n_nodes];

    double time      = 0.0;
    double max_time  = 1.0;
    double dt        = 0.05; 
    int timeStep = 1;
    while(time < max_time)
    {
        UpdateAttr(mesh->n_nodes,time, velocity, pressure);
        
        //CatalystCoProcess(mesh, velocity, pressure,time,timeStep, 0);

        //MeshVTKWriter(mesh, out, timeStep, parts->nodal_part, parts->elem_part, mesh->mesh_coloring_internal, velocity, pressure);

        time += dt;
        timeStep++;
    }



    //CatalystFinalize();
    MeshVTKWriterBin(mesh, out, 0, parts->nodal_part, parts->elem_part, NULL, velocity, pressure);
    //MeshVTKWriterInternalBin(mesh, out, 0, parts->nodal_part, parts->elem_part, mesh->mesh_coloring_internal, velocity, pressure);

    MeshPartitionDestroy(parts);
    MeshDestroy(&mesh);

    return 0;
}


