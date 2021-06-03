#include <iostream>
#include <cmath>
#include <unistd.h>

#include "meshtools_config.h"
#include "mesh.h"
#include "mesh_part.h"
#include "alglin.h"
#include "parallelmesh.h"

#if USE_MPI
#include "mpi.h"
#endif

#ifdef _OPENMP
    #include <omp.h>
#endif

#ifdef USE_CATALYST
#include "FEAdaptor.h"
#endif

using namespace std;

static void usage(const char *arg0)
{
    cerr << "Usage: " << arg0 << " <options>" << endl;
    cerr << "\t -h            : show help" << endl;
    cerr << "\t -m <filename> : mesh filename (gmsh ascii v.2.2)> " << endl;
    exit(-1);
}


int main(int argc, char* argv[])
{         

    int   opt;
    char* gmsh_filename = 0; 
    bool  flg_catalyst     = false;
    bool  flg_gmsh         = false;
    char* catalyst_script = 0;
    

#ifdef USE_MPI
    MPI_Init(argc, argv);
#endif
    if(argc < 2)
    {
        usage(argv[0]);
    }

    while( (opt = getopt(argc, argv, "hm:c:")) !=  -1 ) {
        switch ( opt ) {
            case 'h': /* help */
                usage(argv[0]) ;
                break ;
            case 'm': /* flag -m */
                gmsh_filename = optarg ;
                flg_gmsh      = true;
                break ;
            case 'c':
                catalyst_script = optarg;
                flg_catalyst    = true;
            default:
                fprintf(stderr, "Opcao invalida ou faltando argumento: `%c'\n", optopt) ;
                usage(argv[0]);
                return -1 ;
        }
    }

    if(!flg_gmsh)
    {
        usage(argv[0]);
    }

    int n_processors = 1;
    int processor_id = 0;

#ifdef USE_MPI
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &n_processors);
    MPI_Comm_rank(MPI_COMM_WORLD, &processor_id);
#endif

#ifdef USE_CATALYST
    CatalystInitialize(1, catalyst_script);
#endif

    Mesh             *mesh  = nullptr;
    Mesh_partition_t *parts = nullptr;
    ParallelMesh     *pmesh = nullptr;

    if(processor_id == 0)
    {
        mesh = new Mesh(gmsh_filename);
        mesh->MeshReordering(RCM);
    
        if(n_processors > 1 ) {
            parts = new Mesh_partition_t();
            parts->MeshPartitionerInternal(mesh, n_processors);
        }
    }

    if(n_processors > 1 ) 
    {
        parts->DistributedMeshInternal(mesh);

        std::string str(gmsh_filename);
        str.resize(str.length()-4);
        pmesh = parts->DistributedMeshInternal(mesh);
        pmesh->setFilename(str);
        pmesh->MeshColoring();
        pmesh->writeParallelMesh();
    }
    else
    {
        mesh->MeshColoring();
        mesh->MeshVTKWriterInternalBinAppended(0);
    }

    if(mesh)  delete mesh;
    if(parts) delete parts;
    if(pmesh) delete pmesh;

#ifdef USE_CATALYST
    CatalystFinalize();
#endif

#ifdef USE_MPI
    MPI_Finalize();
#endif

    return 0;

}




