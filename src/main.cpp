#include <iostream>
#include <fstream>
#include <string>
#include <time.h>
#include <math.h>
#include "FEAdaptor.h"

#include "mesh.h"

void UpdateAttr(int n, double time, mesh_t* mesh, double** v, float** p)
{
    float variable_a = 0.02;
    float variable_v = 0.01;

    for(int i = 0; i < n; i++)
    {
        double x = mesh->coord[i*3];
        double y = mesh->coord[i*3+1];

        (*v)[i*3]    = (-1)*cos(variable_a*M_PI*x)*sin(variable_a*M_PI*y)*exp(-2*variable_a*variable_a*M_PI*M_PI*time*variable_v);
        (*v)[i*3+1]  = sin(variable_a*M_PI*x)*cos(variable_a*M_PI*y)*exp(-2*variable_a*variable_a*M_PI*M_PI*time*variable_v);
        (*v)[i*3+2]  = 0;
        (*p)[i]      = -0.25*(cos(2*variable_a*M_PI*x)+cos(2*variable_a*M_PI*y))*exp(-4*variable_a*variable_a*M_PI*M_PI*time*variable_v);
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
        UpdateAttr(mesh->n_nodes, time, mesh, &velocity, &pressure);
        
        CatalystCoProcess(mesh, velocity, pressure,time,timeStep, 0);

        MeshVTKWriterInternalBinAppended(mesh, out, 0, parts->nodal_part, parts->elem_part, mesh->mesh_coloring_internal, velocity, pressure);

        time += dt;
        timeStep++;
    }



    CatalystFinalize();
    //MeshVTKWriterInternalBinAppended(mesh, out, 0, parts->nodal_part, parts->elem_part, mesh->mesh_coloring_internal, velocity, pressure);

    MeshPartitionDestroy(parts);
    MeshDestroy(&mesh);
    delete [] velocity;
    delete [] pressure;

    return 0;
}


