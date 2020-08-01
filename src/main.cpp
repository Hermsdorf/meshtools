#include <iostream>
#include <fstream>
#include <string>
#include <math.h>

#include "FEAdaptor.h"
#include "mesh.h"
#include "mesh_part.h"

void UpdateAttr(int n, double time, Mesh* mesh, double** v, float** p)
{
    float variable_a = 0.02;
    float variable_v = 0.01;

    for(int i = 0; i < n; i++)
    {
        double x = mesh->getCoord()[i*3];
        double y = mesh->getCoord()[i*3+1];

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

    int n_script = 0;
    if(argc == 4)
        n_script = 1;

    //CatalystInitialize(n_script, argv+3);
    
    Mesh* mesh = new Mesh(argv[1]);


    //mesh->MeshReordering(RCM);
    mesh->MeshColoring();


    /*double *velocity = new double[mesh->get_N_nodes()*3];
    float *pressure  = new float[mesh->get_N_nodes()];

    double time      = 0.0;
    double max_time  = 1.0;
    double dt        = 0.05; 
    int timeStep = 1;
    while(time < max_time)
    {
        UpdateAttr(mesh->get_N_nodes(), time, mesh, &velocity, &pressure);
        
        //CatalystCoProcess(mesh, velocity, pressure,time,timeStep, 0);

        //MeshVTKWriterBinAppended(0, parts->get_Nodal_part(), parts->get_Elem_part(), mesh->get_Mesh_coloring_internal(), velocity, pressure);

        time += dt;
        timeStep++;
    }*/

    //CatalystFinalize();
    mesh->MeshVTKWriter(0, NULL, NULL, mesh->get_Mesh_coloring_internal(), NULL, NULL);

    //delete [] velocity;
    //delete [] pressure;
    delete mesh;
    //delete parts;

    return 0;
}


