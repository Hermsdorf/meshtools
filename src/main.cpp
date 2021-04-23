#include <iostream>
#include <cmath>

#include "mpi.h"
#include "mesh.h"
#include "mesh_part.h"
#include "alglin.h"
#include "parallelmesh.h"

using namespace std;

#ifdef _OPENMP
    #include <omp.h>
#endif

#ifdef PARAVIEWCAT_FOUND
   #include "FEAdaptor.h"
#endif

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

void matvec_ebe(Mesh* mesh, Matrix& EBE, double* y, double* r)
{
    unsigned int nnodes = mesh->get_n_nodes();
    unsigned int nelem = mesh->get_n_elements();
    double y_local[8];
    double r_local[8];
    
    std::fill(&r[0], &r[nnodes], 0.0);

    for(int iel = 0 ; iel < nelem ; iel++ )
    {
        unsigned int* conn = mesh->getElementConn(iel);
        unsigned int connsize = mesh->getElementConnSize(iel);

        for(int i = 0 ; i < connsize ; i++)
        {
            unsigned int no = conn[i];
            y_local[i] = y[no];
        }

        std::fill(&r_local[0], &r_local[connsize], 0.0);

        for(int i = 0 ; i < connsize ; i++)
        {
            for(int j = 0 ; j < connsize ; j++)
            {
                r_local[i] += EBE(iel, i, j)*y_local[j];
            }
        }

        for(int i = 0 ; i < connsize ; i++)
        {
            unsigned int no = conn[i];
            r[no] += r_local[i];
        }
    }
}

void matvec_openmp(Mesh* mesh, Matrix& EBE, double* y, double* r)
{
    unsigned int nnodes = mesh->get_n_nodes();
    unsigned int nelem = mesh->get_n_elements();
    double y_local[8];
    double r_local[8];
    
    std::fill(&r[0], &r[nnodes], 0.0);
    
    unsigned int ncolors = mesh->get_n_internal_colors();
    int* elem_color = mesh->get_mesh_coloring_internal();

    unsigned int begin = 0;
    unsigned int end;
    for(int k = 0 ; k < ncolors; k++)
    {
        end = begin + elem_color[k];

        #pragma omp parallel for private(y_local, r_local)
        for(int iel = begin ; iel < end ; iel++)
        {
            unsigned int* conn = mesh->getElementConn(iel);
            unsigned int connsize = mesh->getElementConnSize(iel);

            for(int i = 0 ; i < connsize ; i++)
            {
                unsigned int no = conn[i];
                y_local[i] = y[no];
            }

            std::fill(&r_local[0], &r_local[connsize], 0.0);

            for(int i = 0 ; i < connsize ; i++)
            {
                for(int j = 0 ; j < connsize ; j++)
                {
                    r_local[i] += EBE(iel, i, j)*y_local[j];
                }
            }

            for(int i = 0 ; i < connsize ; i++)
            {
                unsigned int no = conn[i];
                r[no] += r_local[i];
            }
        }
        begin += elem_color[k];
    }
}


int main(int argc, char* argv[])
{         
    if(argc < 2)
    {
        std::cout << "ERROR: WRONG EXECUTION\n";
	    std::cout << "./meshtools filename.msh [catalyst.py]\n";

	    return 0;
    }


    int n_script = 0;
    if(argc == 4)
        n_script = 1;
   
    int n_parts;

    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n_parts);
    
    Mesh* mesh = new Mesh(argv[1]);

#ifdef PARAVIEWCAT_FOUND
        CatalystInitialize(n_script, argv+2);
#endif

    mesh->MeshReordering(RCM);
    // mesh->MeshColoring();

    Mesh_partition_t* parts = new Mesh_partition_t();

    parts->MeshPartitioner(mesh, n_parts);

    ParallelMesh* pmesh = parts->PartitionerMPI(mesh);
    pmesh->writeParallelMesh();
    delete pmesh;

//     double *velocity = new double[mesh->get_n_nodes()*3];
//     float *pressure  = new float[mesh->get_n_nodes()];

//     double time      = 0.0;
//     double max_time  = 1.0;calc
//     double dt        = 0.05; 
//     int timeStep = 1;
//     while(time < max_time)
//     {
//         UpdateAttr(mesh->get_n_nodes(), time, mesh, &velocity, &pressure);

// #ifdef PARAVIEWCAT_FOUND
             //CatalystCoProcess(mesh, velocity, pressure,time,timeStep, 0);
// #endif
         //mesh->MeshVTKWriterInternalBinAppended(timeStep, parts->get_nodal_part(), parts->get_elem_part(), mesh->get_mesh_coloring_internal(), velocity, pressure);

//         time += dt;
//         timeStep++;
//     }

#ifdef PARAVIEWCAT_FOUND
        CatalystFinalize();
#endif

    // unsigned int nelem = mesh->get_n_elements();
    // unsigned int nnodes = mesh->get_n_nodes();
    // unsigned int nconn = mesh->getElementConnSize(0);
    // Matrix ebe(nelem, nconn);
    
    // double* y = new double[nnodes];
    // double* r = new double[nnodes];

    // std::fill(&y[0], &y[nnodes], 1.0);

    // for(int i = 0 ; i < nelem ; i++)
    // {
    //     for(int j = 0 ; j < nconn ; j++)
    //     {
    //         for(int k = 0 ; k < nconn ; k++)
    //         {
    //             ebe(i, j, k) = 1.0;
    //         }
    //     }
    // }

    // matvec_openmp(mesh, ebe, y, r);

    //mesh->MeshVTKWriterInternal(0, NULL, NULL, mesh->get_mesh_coloring_internal(), NULL, NULL);

    // delete [] velocity;
    // delete [] pressure;
    delete mesh;
    delete parts;
    // delete [] y;
    // delete [] r;

    MPI_Finalize();
    return 0;
}




