#include <iostream>
#include <math.h>

#include "FEAdaptor.h"
#include "mesh.h"
#include "mesh_part.h"

void UpdateAttr(int n, double time, Mesh* mesh, double** v, float** p)
{
    float variable_a = 0.02;
    float variable_v = 0.01;

    std::vector<double> coordAux = mesh->getCoord();
    for(int i = 0; i < n; i++)
    {   

        double x = coordAux[i*3];
        double y = coordAux[i*3+1];

        (*v)[i*3]    = (-1)*std::cos(variable_a*M_PI*x)*std::sin(variable_a*M_PI*y)*std::exp(-2*variable_a*variable_a*M_PI*M_PI*time*variable_v);
        (*v)[i*3+1]  = std::sin(variable_a*M_PI*x)*std::cos(variable_a*M_PI*y)*std::exp(-2*variable_a*variable_a*M_PI*M_PI*time*variable_v);
        (*v)[i*3+2]  = 0;
        (*p)[i]      = -0.25*(std::cos(2*variable_a*M_PI*x)+std::cos(2*variable_a*M_PI*y))*std::exp(-4*variable_a*variable_a*M_PI*M_PI*time*variable_v);
    }
}

int main(int argc, char* argv[])
{         
    if(argc < 3)
    {
        std::cout << "ERROR: WRONG EXECUTION\n";
	    std::cout << "./meshtools filename.msh n_partitions [catalyst.py]\n";

	    return 0;
    }

    int n_script = 0;
    if(argc == 4)
        n_script = 1;

    CatalystInitialize(n_script, argv+3);
    int n_part = atoi(argv[2]);

    Mesh* mesh = new Mesh(argv[1]);

    mesh->MeshReordering(RCM);
    mesh->MeshColoring();

    Mesh_partition_t* parts = new Mesh_partition_t;

    parts->MeshPartitioner(mesh, n_part);

    double *velocity = new double[mesh->get_n_nodes()*3];
    float *pressure  = new float[mesh->get_n_nodes()];

    double time      = 0.0;
    double max_time  = 1.0;
    double dt        = 0.05; 
    int timeStep = 1;
    while(time < max_time)
    {
        UpdateAttr(mesh->get_n_nodes(), time, mesh, &velocity, &pressure);
        
        CatalystCoProcess(mesh, velocity, pressure,time,timeStep, 0);

        //mesh->MeshVTKWriterInternalBinAppended(timeStep, parts->get_nodal_part(), parts->get_elem_part(), mesh->get_mesh_coloring_internal(), velocity, pressure);

        time += dt;
        timeStep++;
    }

    CatalystFinalize();
    mesh->MeshVTKWriterBinAppended(0, parts->get_nodal_part(), parts->get_elem_part(), mesh->get_mesh_coloring_internal(), velocity, pressure);

    delete [] velocity;
    delete [] pressure;
    delete mesh;
    delete parts;

    return 0;
}


